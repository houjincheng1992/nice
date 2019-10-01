#include "service.h"

#include "xls.h"

#include "utils/logger.h"

namespace nicer {
void NicerService::check_excel_status(
                        ::google::protobuf::RpcController* controller,
                        const ::nicer::HttpRequest* request,
                        ::nicer::HttpResponse* response,
                        ::google::protobuf::Closure* done) {
    brpc::ClosureGuard guard(done);
    brpc::Controller* ctrl = static_cast<brpc::Controller*>(controller);

    std::string str = cntl->request_attachment().to_string();

    xls_error_t error = LIBXLS_OK;
    xlsWorkBook *wb = xls_open_buffer(str.c_str(), str.size(), "UTF-8", &error);
    if (wb == NULL) {
        printf("Error reading file: %s\n", xls_getError(error));
        exit(1);
    }
    for (int32_t i = 0; i < wb->sheets.count; i++) { // sheets
        xl_WorkSheet *work_sheet = xls_getWorkSheet(work_book, i);
        error = xls_parseWorkSheet(work_sheet);
        for (int32_t j = 0; j <= work_sheet->rows.lastrow; j++) { // rows
            xlsRow *row = xls_row(work_sheet, j);
            for (int32_t k=0; k <= work_sheet->rows.lastcol; k++) { // columns
                xlsCell *cell = &row->cells.cell[k];
                // do something with cell
                if (cell->id == XLS_RECORD_BLANK) {
                    // do something with a blank cell
                } else if (cell->id == XLS_RECORD_NUMBER) {
                   // use cell->d, a double-precision number
                } else if (cell->id == XLS_RECORD_FORMULA) {
                    if (strcmp(cell->str, "bool") == 0) {
                        // its boolean, and test cell->d > 0.0 for true
                    } else if (strcmp(cell->str, "error") == 0) {
                        // formula is in error
                    } else {
                        // cell->str is valid as the result of a string formula.
                    }
                } else if (cell->str != NULL) {
                    // cell->str contains a string value
                }
            }
        }
        xls_close_WS(work_sheet);
    }
    xls_close_WB(wb);

    ctrl->http_response().set_content_type("application/json;charset=utf-8");
    std::string response_str = "{\"haha\":\"nihao\"}";
    INFLOG << "here";
    ctrl->response_attachment().append(response_str);
}
}
