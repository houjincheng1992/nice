#include "service.h"

#include "utils/logger.h"

namespace nicer {
void NicerService::check_excel_status(
                        ::google::protobuf::RpcController* controller,
                        const ::nicer::HttpRequest* request,
                        ::nicer::HttpResponse* response,
                        ::google::protobuf::Closure* done) {
    brpc::ClosureGuard guard(done);
    brpc::Controller* ctrl = static_cast<brpc::Controller*>(controller);
    ctrl->http_response().set_content_type("application/json;charset=utf-8");
    std::string response_str = "{\"haha\":\"nihao\"}";
    INFLOG << "here";
    ctrl->response_attachment().append(response_str);
}
}
