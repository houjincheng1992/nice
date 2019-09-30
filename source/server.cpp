// Copyright (c) 2014 Baidu, Inc.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// A server to receive EchoRequest and send back EchoResponse.

#include <gflags/gflags.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include "proto/http.pb.h"

#include "service.h"

DEFINE_int32(ServicePort, 8805, "服务端口");
DEFINE_int32(MaxConcurrency, 1024, "最大并发度");
DEFINE_int32(MaxLogLength, 4096, "单条日志最大长度");
DEFINE_int32(IdleTimeoutSec, 15, "连接最长空闲时间/s");
DEFINE_int32(verbose, 8, "日志级别");
DEFINE_string(BaseConf, "conf/base.lua", "服务基础配置");
// DEFINE_string(logConf, "conf/log.conf", "日志配置文件路径");

int main(int argc, char* argv[]) {
    // for (uint32_t u = 0; u < argc; ++u) {
    //     if (std::string(argv[u]) == "--version" || std::string(argv[u]) == "-v") {
    //         return print_server_version();
    //     }
    // }

    gflags::ParseCommandLineFlags(&argc, &argv, true);                  //> gflags初始化

    //logging::ComlogSinkOptions options;
    //options.max_log_length = FLAGS_MaxLogLength;
    //options.print_vlog_as_warning = false;
    // int32_t ret = logging::ComlogSink::GetInstance()->SetupFromConfig(FLAGS_logConf);
    // if (ret != 0) {
    //     ERRLOG << "Init log configure failed, server exited";
    //     return ret;
    // }

    brpc::Server server;
    // server.set_version(_SERVER_VERSION);
    int32_t ret = server.AddService(new ::nice::niceService,
                            brpc::SERVER_OWNS_SERVICE,
                            "/fileupload/check_excel_status => check_excel_status");
    if (ret != 0) {
        // ERRLOG << "Fail to add film service";
        return -1;
    }

    brpc::ServerOptions option;
    //option.num_threads = 1;
    option.max_concurrency = FLAGS_MaxConcurrency;
    option.idle_timeout_sec = FLAGS_IdleTimeoutSec;
    if (server.Start(FLAGS_ServicePort, &option) != 0) {
        // ERRLOG << "Fail to start server";
        return -1;
    }

    server.RunUntilAskedToQuit();
    return 0;
}
