﻿//
// Created by etl on 2/5/25.
//


#include "PlottingService.h"

PlottingService::PlottingService(Processor* processor) {
    m_processor = processor;
}

DTOWRAPPERNS::DTOWrapper<ResponseDto::Z__CLASS> PlottingService::processPlotting(
        const oatpp::String& token,
        const oatpp::web::server::api::ApiController::Object<PlottingDto>& plottingDto) {
    // Implement the actual plotting logic here
    spdlog::debug("debug Processing plotting request");

    // 创建一个 JSON ObjectMapper
#if OATPP_VERSION_LESS_1_4_0
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
#else
    auto objectMapper = std::make_shared<oatpp::json::ObjectMapper>();
#endif
    // 将 plottingDto 序列化为 JSON 字符串
    std::string jsonStr;
    try {
        jsonStr = objectMapper->writeToString(plottingDto);
    } catch (const std::exception& e) {
        spdlog::error("Error serializing plottingDto to JSON: {}", e.what());
        // 这里可以根据需要进行错误处理，例如返回错误响应
    }

    spdlog::debug("processPlotting Processing plotting request, requestBody: {}", QString::fromStdString(jsonStr).toUtf8());

    auto responseDto = m_processor->processByPlottingWeb(token, plottingDto).get();

    spdlog::debug("processPlotting Processing plotting response, responseDto -> msg: {}", responseDto->error->c_str());

    return responseDto;
}