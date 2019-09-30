/* Copyright (c) 2017 Baidu.com, Inc. All Rights Reserved
 * *********************************************************
 * @file service.h
 * @author houjincheng
 * @date 2017.05.07
 * @version
 * @brief
 **/
#pragma once

#include <brpc/server.h>

#include "proto/http.pb.h"

namespace nice {

class NiceService : public ::nice::HttpService {
public:
    NiceService() {};
    ~NiceService() {};

    virtual void check_excel_status(::google::protobuf::RpcController* controller,
                        const ::nice::HttpRequest* request,
                        ::nice::HttpResponse* response,
                        ::google::protobuf::Closure* done);
};

}//namespace nice
/* Powered by Microsoft Visual Studio 2013, @2017*/
