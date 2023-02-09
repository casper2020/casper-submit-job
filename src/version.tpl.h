/**
* @file version.h
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
* along with casper.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef CASPER_SUBMIT_JOB_VERSION_H_
#define CASPER_SUBMIT_JOB_VERSION_H_

#ifndef CASPER_SUBMIT_JOB_ABBR
#define CASPER_SUBMIT_JOB_ABBR "csj"
#endif

#ifndef CASPER_SUBMIT_JOB_NAME
#define CASPER_SUBMIT_JOB_NAME "submit-job@b.n.s@"
#endif

#ifndef CASPER_SUBMIT_JOB_VERSION
#define CASPER_SUBMIT_JOB_VERSION "x.x.x"
#endif

#ifndef CASPER_SUBMIT_JOB_REL_DATE
#define CASPER_SUBMIT_JOB_REL_DATE "r.r.d"
#endif

#ifndef CASPER_SUBMIT_JOB_REL_BRANCH
#define CASPER_SUBMIT_JOB_REL_BRANCH "r.r.b"
#endif

#ifndef CASPER_SUBMIT_JOB_REL_HASH
#define CASPER_SUBMIT_JOB_REL_HASH "r.r.h"
#endif

#ifndef CASPER_SUBMIT_JOB_REL_TARGET
#define CASPER_SUBMIT_JOB_REL_TARGET "r.r.t"
#endif

#ifndef CASPER_SUBMIT_JOB_INFO
#define CASPER_SUBMIT_JOB_INFO CASPER_SUBMIT_JOB_NAME " v" CASPER_SUBMIT_JOB_VERSION
#endif

#define CASPER_SUBMIT_JOB_BANNER \
  "   ___    _   _   _____   _   _   _____         _    ___    ____  " \
  "\n  / _ \\  | | | | | ____| | | | | | ____|       | |  / _ \\  | __ ) " \
  "\n | | | | | | | | |  _|   | | | | |  _|      _  | | | | | | |  _ \\ " \
  "\n | |_| | | |_| | | |___  | |_| | | |___    | |_| | | |_| | | |_) |" \
  "\n  \\__\\_\\  \\___/  |_____|  \\___/  |_____|    \\___/   \\___/  |____/ "

#endif // CASPER_SUBMIT_JOB_VERSION_H_
