/* Copyright (c) 2017 houjincheng1992, Inc. All Rights Reserved
 * *********************************************************
 * @file log.h
 * @author houjincheng1992@163.com
 * @date 2019.04.14
 * @version
 * @brief
 **/
#pragma once

#include <stdint.h>
#include <iomanip>
#include <butil/logging.h>

#define HEX(num) std::hex<<num<<std::dec
#define JOB(idx) "JOB["<<HEX(idx)<<"] "

#define DBGLOG LOG(DEBUG)
#define INFLOG LOG(INFO)
#define NOTICE LOG(NOTICE)
#define WARLOG LOG(WARNING)
#define ERRLOG LOG(ERROR)

#define DBGLOGWITHID(id) DBGLOG<<JOB(id)
#define INFLOGWITHID(id) INFLOG<<JOB(id)
#define NOTICEWITHID(id) NOTICE<<JOB(id)
#define WARLOGWITHID(id) WARLOG<<JOB(id)
#define ERRLOGWITHID(id) ERRLOG<<JOB(id)

/* Powered by Microsoft Visual Studio 2013, @2019*/
