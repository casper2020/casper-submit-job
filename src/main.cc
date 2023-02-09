/**
* @file main.cc
*
* Copyright (c) 2011-2023 Cloudware S.A. All rights reserved.
*
* This file is part of casper-submit-job.
*
* casper-submit-job is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* casper-submit-job is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with casper. If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>

#include "version.h"

#include "cc/optarg.h"
#include "cc/exception.h"
#include "cc/threading/worker.h"

#include "cc/easy/redis.h"
#include "cc/easy/beanstalk.h"

/**
 * @brief Main.
 *
 * param argc
 * param argv
 *
 * return
 */
int main(int argc, char** argv)
{
    try {
        
        const std::string default_redis_conn_str      = std::string(cc::easy::Redis::k_ip_addr_) + ':' + std::to_string(cc::easy::Redis::k_port_nbr_);
        const std::string default_beanstalkd_conn_str = std::string(cc::easy::Beanstalk::k_ip_addr_) + ':' + std::to_string(cc::easy::Beanstalk::k_port_nbr_);
        const uint64_t    default_job_ttr             = 3600;
        const uint64_t    default_job_validity        = 7200;
        
        std::map<std::string, std::string> extra_keys = {};
        
        const auto split_string = [] (const char* const a_key, const std::string& a_value, char a_delimiter = ':') -> std::tuple<std::string, std::string> {
            std::stringstream ss(a_value);
            std::string       token;
            std::vector<std::string> elements;
            while (std::getline(ss, token, a_delimiter)) {
                elements.push_back(std::move(token));
            }
            if ( elements.size() != 2 ) {
                throw cc::Exception("Unexpected number of arguments while splitting %s value %s, got " SIZET_FMT ", expected " SIZET_FMT "!",
                                    a_key, a_value.c_str(), elements.size(), static_cast<size_t>(2)
                );
            }
            return std::make_tuple(elements[0], elements[1]);
        };
    
        const auto split_conn_str = [&split_string] (const char* const a_key, const std::string& a_value, char a_delimiter = ':') -> std::tuple<std::string, int> {
            // ... split as string ...
            auto tuple = split_string(a_key, a_value, a_delimiter);
            // ... convert value to number and we're done
            return std::make_tuple(std::get<0>(tuple), std::atoi(std::get<1>(tuple).c_str()));
        };


        cc::OptArg opt (CASPER_SUBMIT_JOB_NAME, CASPER_SUBMIT_JOB_VERSION,
                        CASPER_SUBMIT_JOB_REL_DATE, CASPER_SUBMIT_JOB_REL_BRANCH, CASPER_SUBMIT_JOB_REL_HASH,
                        CASPER_SUBMIT_JOB_BANNER,
            {
                // NOTES: THIS OBJECTS WILL BE DELETE WHEN cc::OptArg::~OptArg IS CALLED
                new cc::OptArg::String(/* a_long */ "redis", /* a_short */ 'r',
                                       /* a_default */ (const std::string&)default_redis_conn_str , /* a_tag */ "HOST:PORT",
                                       /* a_help */
                                       ( "Hostname or ip address and port of the REDIS server (default: " +  default_redis_conn_str +  ")." ).c_str()
                ),
                new cc::OptArg::String(/* a_long */ "beanstalkd", /* a_short */ 'b',
                                       /* a_default */ (const std::string&)default_beanstalkd_conn_str , /* a_tag */ "HOST:PORT",
                                       /* a_help */
                                       ( "Hostname or ip address and port of the BEANSTALKD server (default: " +  default_beanstalkd_conn_str +  ")." ).c_str()
                ),
                new cc::OptArg::String(/* a_long */ "sid", /* a_short */ 'i',
                                       /* a_optional */ false , /* a_tag */ "id",
                                       /* a_help */
                                       "REDIS service ID."
                ),
                new cc::OptArg::String(/* a_long */ "tube", /* a_short */ 't',
                                       /* a_optional */ false , /* a_tag */ "name",
                                       /* a_help */
                                       "BEANSTALKD tube name."
                ),
                new cc::OptArg::UInt64(/* a_long */ "ttr", /* a_short */ 'v',
                                       /* a_default */ (const uint64_t&)default_job_ttr, /* a_tag */ "seconds",
                                       /* a_help */
                                       ( "Job time to run in SECONDS (default: " + std::to_string(default_job_ttr) + ")." ).c_str()
                ),
                new cc::OptArg::UInt64(/* a_long */ "validity", /* a_short */ 'V',
                                       /* a_default */ (const uint64_t&)default_job_validity, /* a_tag */ "seconds",
                                       /* a_help */
                                       ( "Job validty in SECONDS (default: " + std::to_string(default_job_validity) + ")." ).c_str()
                ),
                new cc::OptArg::String(/* a_long */ "payload", /* a_short */ 'p',
                                       /* a_optional */ true, /* a_tag */ "json",
                                       /* a_help */
                                       "Job payload JSON as quoted string."
                ),
                new cc::OptArg::Switch(/* a_long */ "help", /* a_short */ 0,
                                       /* a_optional */ (const bool)true, /* a_help */ "Show help."
                ),
                new cc::OptArg::Switch(/* a_long */ "version", /* a_short */ 0,
                                       /* a_optional */ (const bool)true, /* a_help */ "Show version."
                ),
                new cc::OptArg::Switch(/* a_long */ "debug", /* a_short */ 'd',
                                       /* a_optional */ (const bool)true, /* a_help */ "Developer mode: log to stdout and print job."
                )
            }
        );
        
        const int argp = opt.Parse(argc, const_cast<const char** const >(argv),
            /* a_unknown_argument_callback */
            [&extra_keys, &split_string] (const char* const a_key, const char* const a_value) -> bool {
                // ... accept?
                if ( nullptr == a_key || nullptr == a_value ) {
                    // .. 2nd attempt ...
                    if ( nullptr != a_key && nullptr == a_value ) {
                        // ... split by '=' ...
                        const auto tuple = split_string(a_key, a_key, '=');
                        // ... keep track of it ...
                        extra_keys[std::get<0>(tuple)] = std::get<1>(tuple);
                        // ... accepted  ...
                        return true;
                    }
                    // ... rejected ...
                    return false;
                }
                // ... no, keep track if it ...
                extra_keys[a_key] = a_value;
                // ... accepted  ...
                return true;
            },
            /* a_special_switch_callback */
            [&opt] () -> bool {
                // ... is it a special switch?
                if ( true == opt.IsSet("help") || true == opt.IsSet("about") ) {
                    // ... yes ...
                    return true;
                }
                // ... no, it's an error ...
                return false;
            }
        );

        // ... help?
        if ( true == opt.IsSet("help") ) {
            opt.ShowHelp();
            return 0;
        }
        
        // ... about // version ?
        if ( true == opt.IsSet("version") ) {
            opt.ShowVersion();
            return 0;
        }
        
        // ... missing or invalid arguments ...
        if ( 0 != argp ) {
            opt.ShowHelp(opt.error());
            return -1;
        }
               
        // ... set thread name ...
        ::cc::threading::Worker::SetName(CASPER_SUBMIT_JOB_NAME);

        //
        // COMMON
        //
 
        long long         job_id       = -1;
        const std::string job_sid      = opt.GetString('i')->value();
        const std::string job_tube     = opt.GetString('t')->value();
        const std::string job_id_key   = job_sid + ":jobs:sequential_id";
              std::string job_key      = ( job_sid + ":jobs:" + job_tube + ':' );
        const size_t      job_ttr      = static_cast<size_t>(opt.GetUInt64ValueOf('v'));
        const size_t      job_validity = static_cast<size_t>(opt.GetUInt64ValueOf('V'));
        const auto        r_c_s        = split_conn_str("redis HOST:PORT", opt.GetStringValueOf('r'));
        const auto        b_c_s        = split_conn_str("beanstalkd HOST:PORT", opt.GetStringValueOf('b'));

        //
        // DEBUG // LOG PURPOSES
        //
        if ( true == opt.IsSet('d') ) {
            fprintf(stdout, "%s v%s [ Rel.Date: %s ]\n", CASPER_SUBMIT_JOB_NAME, CASPER_SUBMIT_JOB_VERSION, CASPER_SUBMIT_JOB_REL_DATE);
            fprintf(stdout, "arguments:\n");
            fprintf(stdout, "\t --%-8s: %s\n"           , "sid"      , job_sid.c_str());
            fprintf(stdout, "\t --%-8s: " SIZET_FMT" \n", "ttr"      , job_ttr);
            fprintf(stdout, "\t --%-8s: " SIZET_FMT "\n", "validity" , job_validity);
            fprintf(stdout, "\t --%-8s: %s:%d\n"        , "REDIS"    , std::get<0>(r_c_s).c_str(), std::get<1>(r_c_s) );
            fprintf(stdout, "\t --%-8s: %s:%d\n"        , "BEANSTALD", std::get<0>(b_c_s).c_str(), std::get<1>(b_c_s) );
            if ( extra_keys.size() > 0 ) {
                fprintf(stdout, "other:\n");
                for ( auto p : extra_keys ) {
                    fprintf(stdout, "\t  %-8s: %s\n", p.first.c_str(), p.second.c_str());
                }
            }
            fflush(stdout);
        }
        
        //
        // REDIS
        //
        {
            ::cc::easy::Redis redis;
                        
            redis.Connect(std::get<0>(r_c_s), std::get<1>(r_c_s));
            
            job_id   = redis.INCR(job_id_key);
            job_key += std::to_string(job_id);
            
            redis.HSET(job_key, "status", "{\"status\":\"queued\"}");
            redis.EXPIRE(job_key, job_validity);

            redis.Disconnect();
        }
        
        //
        // BEANSTALKD
        //
        {
            cc::easy::Beanstalk beanstak(cc::easy::Beanstalk::Mode::Producer);
                    
            beanstak.Connect(std::get<0>(b_c_s), std::get<1>(b_c_s), { job_tube }, static_cast<float>(0));
            if ( true == opt.IsSet('p') ) {
                beanstak.Push(std::to_string(job_id), opt.GetStringValueOf('p'), extra_keys, job_ttr, job_validity);
            } else {
                beanstak.Push(std::to_string(job_id), extra_keys, job_ttr, job_validity);
            }
            beanstak.Disconnect();
        }
        
        //
        // DEBUG // LOG PURPOSES
        //
        if ( true == opt.IsSet('d') ) {
            fprintf(stdout, "finished:\n");
            fprintf(stdout, "\t  %s\n", job_key.c_str());
            fflush(stdout);
        }

    } catch (const ::cc::Exception& a_cc_exception ) {
        fprintf(stderr, "%s\n", a_cc_exception.what());
        fflush(stderr);
    } catch (...) {
        try {
            ::cc::Exception::Rethrow(/* a_unhandled */ true, __FILE__, __LINE__, __FUNCTION__);
        } catch (const ::cc::Exception& a_cc_exception) {
            fprintf(stderr, "%s\n", a_cc_exception.what());
            fflush(stderr);
        }
    }
    
}
