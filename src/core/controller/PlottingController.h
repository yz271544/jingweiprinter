﻿//
// Created by Lyndon on 2025/1/29.
//

#ifndef PLOTTINGCONTROLLER_H
#define PLOTTINGCONTROLLER_H

#include <oatpp/codegen/api_controller/base_define.hpp>
#include <oatpp/network/Server.hpp>
#include <oatpp/web/server/HttpConnectionHandler.hpp>
#include <oatpp/web/server/HttpRouter.hpp>
#include <spdlog/spdlog.h>
#if OATPP_VERSION_LESS_1_4_0
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/core/Types.hpp>
#else
#include <oatpp/macro/codegen.hpp>
#include <oatpp/json/ObjectMapper.hpp>
#include <oatpp/Types.hpp>
#endif
#include "core/handler/dto/plotting.h"
#include "core/service/PlottingService.h"

#include OATPP_CODEGEN_BEGIN(ApiController)
// 定义POST接口处理函数
class PlottingController : public oatpp::web::server::api::ApiController {
private:
    PlottingService* m_plottingService; // 业务逻辑服务类
public:
    PlottingController(std::shared_ptr<OBJECTMAPPERNS::ObjectMapper>& objectMapper,
                       oatpp::String& routePrefix,
                       PlottingService* plottingService);

    static std::shared_ptr<PlottingController> createShared(std::shared_ptr<OBJECTMAPPERNS::ObjectMapper>& objectMapper,
                                                            oatpp::String& routePrefix,
                                                            PlottingService* plottingService);
    ENDPOINT_INFO(plotting) {
        info->summary = "Plotting endpoint";
        info->addConsumes<Object<PlottingDto>>("application/json");
        info->addResponse<Object<ResponseDto>>(Status::CODE_200, "application/json");
    }

    ENDPOINT("POST", "/qgz", plotting,
             REQUEST(std::shared_ptr<IncomingRequest>, request),
             BODY_DTO(Object<PlottingDto>, plottingDto)) {
        auto token = request->getHeader("Authorization");
        spdlog::debug("Authorization header: {}", token->c_str());
        // 调用业务逻辑服务类处理请求
        auto responseDto = m_plottingService->processPlotting(token, plottingDto);
        // 返回响应
#if OATPP_VERSION_LESS_1_4_0
        return createDtoResponse(Status::CODE_200, responseDto, this->getDefaultObjectMapper());
#else
        return createDtoResponse(Status::CODE_200, responseDto, this->getContentMappers()->getDefaultMapper());
#endif
    }
};

#include OATPP_CODEGEN_END(ApiController)

#endif //PLOTTINGCONTROLLER_H
