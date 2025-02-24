﻿//
// Created by etl on 2/5/25.
//

#ifndef JINGWEIPRINTER_PLOTTINGSERVICE_H
#define JINGWEIPRINTER_PLOTTINGSERVICE_H

#include <spdlog/spdlog.h>
#include <memory>
#include <oatpp/web/server/api/ApiController.hpp>

#include "core/handler/dto/plotting.h"
#include "core/qgis/Processor.h"
#include "config.h"

class PlottingService {
private:
    Processor* m_processor;
public:
    PlottingService(Processor* processor);

    ~PlottingService() = default;

    // 处理绘图逻辑
    DTOWRAPPERNS::DTOWrapper<ResponseDto::Z__CLASS> processPlotting(
            const oatpp::String& token,
            const oatpp::web::server::api::ApiController::Object<PlottingDto>& plottingDto);
};

#endif //JINGWEIPRINTER_PLOTTINGSERVICE_H
