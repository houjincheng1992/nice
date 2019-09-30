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

namespace nicer {

class NicerService : public ::nicer::HttpService {
public:
    NicerService() {};
    ~NicerService() {};

    virtual void check_excel_status(::google::protobuf::RpcController* controller,
                        const ::nicer::HttpRequest* request,
                        ::nicer::HttpResponse* response,
                        ::google::protobuf::Closure* done);
};

}//namespace nicer
/* Powered by Microsoft Visual Studio 2013, @2017*/
