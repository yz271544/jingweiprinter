//
// Created by etl on 2/7/25.
//

#include "Processor.h"

Processor::Processor(const QList<QString> &argvList, YAML::Node *config) {
    m_config = config;
    try {
        m_verbose = m_config->operator[]("logging")["verbose"].as<bool>();
    } catch (const std::exception &e) {
        spdlog::warn("get verbose error: {}", e.what());
    }
    try {
        m_enable_3d = m_config->operator[]("qgis")["enable_3d"].as<bool>();
    } catch (const std::exception &e) {
        spdlog::warn("get m_enable_3d error: {}", e.what());
    }
    try {
        m_export_prefix = QString::fromStdString((*m_config)["qgis"]["export_prefix"].as<std::string>());
    } catch (const std::exception &e) {
        spdlog::warn("get qgis.export_prefix error: {}", e.what());
    }
    QString jingwei_server_host = "127.0.0.1";
    try {
        jingwei_server_host = QString::fromStdString((*m_config)["qgis"]["jingwei_server_host"].as<std::string>());
    } catch (const std::exception &e) {
        spdlog::warn("get jingwei_server_host error: {}", e.what());
    }
    std::int32_t jingwei_server_port = 8080;
    try {
        jingwei_server_port = (*m_config)["qgis"]["jingwei_server_port"].as<std::int32_t>();
    } catch (const std::exception &e) {
        spdlog::warn("get jingwei_server_port error: {}", e.what());
    }
    QString jingwei_server_api_prefix = "/api";
    try {
        jingwei_server_api_prefix = QString::fromStdString(
                (*m_config)["qgis"]["jingwei_server_api_prefix"].as<std::string>());
    } catch (const std::exception &e) {
        spdlog::warn("get jingwei_server_api_prefix error: {}", e.what());
    }
    QString jingwei_server_url = "";
    try {
        jingwei_server_url = QString::fromStdString((*m_config)["qgis"]["jingwei_server_url"].as<std::string>());
    } catch (const std::exception &e) {
        spdlog::warn("get jingwei_server_url error: {}", e.what());
    }
    try {
        m_export_png_enable = m_config->operator[]("qgis")["export_png_enable"].as<bool>();
    } catch (const std::exception &e) {
        spdlog::warn("get export_png_enable error: {}", e.what());
    }
    try {
        m_export_pdf_enable = m_config->operator[]("qgis")["export_pdf_enable"].as<bool>();
    } catch (const std::exception &e) {
        spdlog::warn("get export_pdf_enable error: {}", e.what());
    }
    try {
        m_export_svg_enable = m_config->operator[]("qgis")["export_svg_enable"].as<bool>();
    } catch (const std::exception &e) {
        spdlog::warn("get export_svg_enable error: {}", e.what());
    }
    jingwei_server_url = jingwei_server_url.replace("{JINGWEI_SERVER_HOST}", jingwei_server_host);
    jingwei_server_url = jingwei_server_url.replace("{JINGWEI_SERVER_PORT}", QString::number(jingwei_server_port));
    jingwei_server_url = jingwei_server_url.replace("{JINGWEI_SERVER_API_PREFIX}", jingwei_server_api_prefix);
//    spdlog::info("jingwei_server_url: {}", jingwei_server_url.toStdString());

    QString mapping_export_nginx_host = "localhost";
    try {
        auto envExportNginxHost = getEnvString("MAPPING_EXPORT_NGINX_HOST");
        if (!envExportNginxHost.empty()) {
            mapping_export_nginx_host = QString::fromStdString(envExportNginxHost);
        } else {
            auto confExportNginxHost = (*m_config)["qgis"]["mapping_export_nginx_host"];
            if (confExportNginxHost) {
                mapping_export_nginx_host = QString::fromStdString(confExportNginxHost.as<std::string>());
            }
        }
    } catch (const std::exception &e) {
        spdlog::warn("get mapping_export_nginx_host error: {}", e.what());
    }

    std::int32_t mapping_export_nginx_port = 80;
    try {
        auto envExportNginxPort = getEnvInt32("MAPPING_EXPORT_NGINX_PORT");
        if (envExportNginxPort) {
            mapping_export_nginx_port = envExportNginxPort;
        } else {
            auto confExportNginxPort = (*m_config)["qgis"]["mapping_export_nginx_port"];
            if (confExportNginxPort) {
                mapping_export_nginx_port = confExportNginxPort.as<std::int32_t>();
            }
        }
    } catch (const std::exception &e) {
        spdlog::warn("get mapping_export_nginx_port error: {}", e.what());
    }

    try {
        auto confMappingExportNginxUrlPrefix = (*m_config)["qgis"]["mapping_export_nginx_url_prefix"];
        if (confMappingExportNginxUrlPrefix) {
            m_mapping_export_nginx_url_prefix = QString::fromStdString(
                    confMappingExportNginxUrlPrefix.as<std::string>());
            m_mapping_export_nginx_url_prefix = m_mapping_export_nginx_url_prefix.replace("{MAPPING_EXPORT_NGINX_HOST}",
                                                                                          mapping_export_nginx_host);
            m_mapping_export_nginx_url_prefix = m_mapping_export_nginx_url_prefix.replace("{MAPPING_EXPORT_NGINX_PORT}",
                                                                                          QString::number(
                                                                                                  mapping_export_nginx_port));
        }
//        spdlog::info("mapping_export_nginx_url_prefix: {}", m_mapping_export_nginx_url_prefix.toStdString());
    } catch (const std::exception &e) {
        spdlog::warn("get mapping_export_nginx_url_prefix error: {}", e.what());
    }

    m_plotting_fetch = std::make_unique<PlottingFetch>(jingwei_server_url.toStdString());

    m_app = std::make_unique<App>(argvList, m_config);
    if (!m_app) {
        spdlog::error("m_app is nullptr!");
    }

    // 读取图像规格
    m_setting_image_spec = std::make_unique<QVariantMap>();
//    spdlog::info("read the image spec");
    auto specification = (*m_config)["specification"];
    QList<QVariant> specList = NodeToMap::sequenceToVariantList(specification);
    for (const auto &item: specList) {
        auto itemMap = item.toMap();
        (*m_setting_image_spec)[itemMap["name"].toString()] = itemMap;
    }
    if (m_verbose) {
        QVariantMap::Iterator it;
        //for (const auto &key: m_setting_image_spec->keys()) {
        for (it = m_setting_image_spec->begin(); it != m_setting_image_spec->end(); ++it) {
            auto const &key = it.key();
            spdlog::debug("image_spec: {}", key.toStdString());
            auto value = m_setting_image_spec->value(key).toMap();
            spdlog::debug("local: {}", value["local"].toString().toStdString());
        }
    }
}

Processor::~Processor() {
//    spdlog::info("Processor destroyed");
}

std::future<DTOWRAPPERNS::DTOWrapper<PlottingRespDto>>
Processor::fetchPlotting(const oatpp::String &token, const oatpp::String &scene_type,
                         DTOWRAPPERNS::DTOWrapper<TopicMapData> &topic_map_data) {
    // 使用 std::async 来实现异步操作
    return std::async(std::launch::async, [this, token, scene_type, topic_map_data]() {
        // 环境配置 ENV_PROFILE=test 时，使用测试数据
        const char *envProfile = getenv("ENV_PROFILE");
        if (envProfile != nullptr) {
            std::string profile(envProfile);
            if (profile == "test") {
//                spdlog::info("ENV_PROFILE: {}", profile);
                // open the file and read the json
                QFile file("/lyndon/iProject/cpath/jingweiprinter/common/input/topicMap.json");
                if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    spdlog::error("Cannot open file: {}", file.errorString().toStdString());
                }

                QTextStream in(&file);
                QString jsonContent = in.readAll();
                file.close();

                auto objectMapper = std::make_shared<OBJECTMAPPERNS::ObjectMapper>();

                try {
                    auto plottingRespDto = objectMapper->readFromString<oatpp::Object<PlottingRespDto>>(
                            jsonContent.toStdString().c_str());

                    if (!plottingRespDto) {
                        spdlog::error("Failed to parse JSON!");
                    }
                    return plottingRespDto;
                } catch (const std::exception &e) {
                    spdlog::error("Failed to parse JSON: {}", e.what());
                }
            }
        }


        // 发送请求获取绘图数据
        m_plotting_fetch->setToken(token);
        std::unordered_map<oatpp::String, oatpp::String> additionalHeaders = {
                {"sceneType", scene_type}
        };
        auto resp = m_plotting_fetch->fetch(additionalHeaders, topic_map_data);
        return resp;
    });
}


// 异步处理绘图数据的函数
std::future<DTOWRAPPERNS::DTOWrapper<ResponseDto>>
Processor::processByPlottingWeb(const oatpp::String &token, const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingWeb) {

    // 创建一个 std::promise 用于存储返回值
    auto promise = std::make_shared<std::promise<DTOWRAPPERNS::DTOWrapper<ResponseDto>>>();

    // 使用 std::async 来实现异步操作
    return std::async(std::launch::async, [this, token, plottingWeb, promise]() {

        // 创建事件循环
        QEventLoop eventLoop;

        if (m_verbose) {
            QJsonDocument postPlottingWebBody = JsonUtil::convertDtoToQJsonObject(plottingWeb);
            spdlog::debug("input plottingWeb: {}",
                          postPlottingWebBody.toJson(QJsonDocument::JsonFormat::Compact).toStdString());
        }

        // 发送请求get绘图数据
        auto topicMapData = TopicMapData::createShared();
        topicMapData->sceneId = plottingWeb->sceneId;
        // check and closed the polygon
        checkDealWithClosedGeometry(plottingWeb->geojson);
        auto scopeJson = JsonUtil::convertDtoToQJsonObject(plottingWeb->geojson);
        topicMapData->scope = scopeJson.toJson(QJsonDocument::JsonFormat::Compact).toStdString();

        QString layoutType = "现场位置图";
        if (plottingWeb->topicCategory && !plottingWeb->topicCategory->empty()) {
            topicMapData->topicCategory = plottingWeb->topicCategory->c_str();
            if (plottingWeb->topicCategory.equalsCI_ASCII("02")) {
                layoutType = "警力部署图";
            } else if (plottingWeb->topicCategory.equalsCI_ASCII("03")) {
                layoutType = "紧急疏散图";
            }
        } else {
            topicMapData->topicCategory = "";
        }
        // show XServer Request body
        if (m_verbose) {
            auto topicMapDataJson = JsonUtil::convertDtoToQJsonObject(topicMapData);
            spdlog::debug("topicMapData: {}",
                          topicMapDataJson.toJson(QJsonDocument::JsonFormat::Compact).toStdString());
        }

        // 获取 XServer 绘图数据
        auto plottingRespDto = fetchPlotting(token, plottingWeb->sceneType, topicMapData).get();
        if (m_verbose) {
            auto plottingRespDtoJson = JsonUtil::convertDtoToQJsonObject(plottingRespDto);
            spdlog::debug("plottingRespDtoJson: {}",
                          plottingRespDtoJson.toJson(QJsonDocument::JsonFormat::Compact).toStdString());
        }

        if (plottingRespDto->code >= 400) {
            std::string errorMsg = fmt::format("fetch plotting data error: {}", plottingRespDto->msg->c_str());
            spdlog::error(errorMsg);
            throw XServerRequestError(errorMsg);
        }
//        spdlog::info("invoke method to create project");
        // Qt::TimerType tempReceiver;
        QMetaObject::invokeMethod(qApp, [this, plottingWeb, plottingRespDto, layoutType, promise, &eventLoop]() {
            //QTimer::singleShot(0, tempReceiver, [this, plottingWeb, plottingRespDto, layoutType, promise, &eventLoop]() {
//            spdlog::info("Inside invokeMethod lambda: start");
            auto responseDto = ResponseDto::createShared();
            try {
                QString sceneName = QString::fromStdString(plottingWeb->sceneName);
                spdlog::debug("create qgis project, sceneName: {}", sceneName.toStdString());
                QString mainCrs = QString::fromStdString(MAIN_CRS);
                spdlog::debug("mainCrs: {}", mainCrs.toStdString());

                // 使用 QMetaObject::invokeMethod 来确保在 App 对象所在的线程中调用 createProjectSlot
                /*QMetaObject::invokeMethod(m_app.get(), "createProjectSlot", Qt::QueuedConnection,
                                          Q_ARG(QString, sceneName), Q_ARG(QString, mainCrs));*/

                m_app->createProject(sceneName, mainCrs);
                spdlog::debug("add map base tile layer");
                m_app->addMapBaseTileLayer();

                if (!plottingWeb->path->empty()) {
                    QString plottingWebPaths = QString::fromStdString(*plottingWeb->path);
                    QStringList orthogonal_paths = plottingWebPaths.split(",");
                    for (int i = 0; i < orthogonal_paths.size(); ++i) {
                        QString orthogonal_path = orthogonal_paths[i];
                        if (orthogonal_path.contains("/")) {
                            orthogonal_path = orthogonal_path.split("/").last();
                        } else if (orthogonal_path.contains("\\")) {
                            orthogonal_path = orthogonal_path.split("\\").last();
                        }
                        QString plottingWebSceneId = QString::fromStdString(*plottingWeb->sceneId);
                        orthogonal_path = plottingWebSceneId.append("-").append(orthogonal_path.trimmed());
                        m_app->addMapMainTileLayer(i, orthogonal_path);
                    }
                }

                //auto path3dProp = plottingWeb->Z__PROPERTY_INITIALIZER_PROXY_path3d();
                //if (path3dProp.getPtr() && !plottingWeb->path3d->empty()) {
                if (!plottingWeb->path3d->empty()) {
                    QString plottingWebPath3ds = QString::fromStdString(*plottingWeb->path3d);
                    QStringList real_3d_paths = plottingWebPath3ds.split(",");
                    for (int i = 0; i < real_3d_paths.size(); ++i) {
                        QString path3d = real_3d_paths[i].trimmed();
                        QStringList path3d_arr = path3d.split("/");
                        if (path3d_arr.last() == "tileset.json") {
                            path3d = path3d_arr[path3d_arr.size() - 2] + "/" + path3d_arr.last();
                        }
                        QString plottingWebSceneId = QString::fromStdString(*plottingWeb->sceneId);
                        QString real_3d_path = plottingWebSceneId.append("-").append(path3d);
                        m_app->addMap3dTileLayer(i, real_3d_path);
                    }
                }

                // add layers
                plottingLayers(plottingRespDto);

                // create 2d canvas
                m_app->createCanvas(mainCrs);
                auto extent = m_app->resetCanvas(plottingWeb->geojson);
                qDebug() << "refresh extent -> width: " << extent.width() << " height: " << extent.height()
                         << " xMin: " << QString::number(extent.xMinimum(), 'f', 15)
                         << " xMax: " << QString::number(extent.xMaximum(), 'f', 15)
                         << " yMin: " << QString::number(extent.yMinimum(), 'f', 15)
                         << " yMax: " << QString::number(extent.yMaximum(), 'f', 15);
                QgsReferencedRectangle referenced_rectangle(extent, m_app->getProject()->crs());
                m_app->getProject()->viewSettings()->setDefaultViewExtent(referenced_rectangle);

                auto image_spec = m_setting_image_spec->value(layoutType).toMap();

                if (!image_spec.isEmpty()) {
                    bool is3D = false;
                    try {
                        is3D = plottingWeb->mapType->is3D;
                    } catch (const std::exception &e) {
                        spdlog::warn("get mapType.is3D error: {}", e.what());
                    }
                    if (is3D) {
                        spdlog::info("add and export 3d layout");
                        if (m_enable_3d) {
                            // add 3d layout
                            spdlog::debug("add 3d layout");
                            QVector<QString> removeLayerNames3D = QVector<QString>();
                            QVector<QString> removeLayerPrefixes3D = QVector<QString>();
                            removeLayerNames3D.append(BASE_TILE_NAME);
                            removeLayerPrefixes3D.append(MAIN_TILE_NAME);
                            auto paperSpec = plottingWeb->paper;
                            auto upperPaperSpec = Formula::toUpperCase(paperSpec);
                            PaperSpecification availablePaper3D(QString::fromStdString(upperPaperSpec));
                            auto canvas2d = m_app->getCanvas();

                            // 设置 OpenGL 表面格式
                            QSurfaceFormat format;
                            format.setVersion(4, 1);  // 设置 OpenGL 版本
                            format.setProfile(QSurfaceFormat::CoreProfile);
                            QSurfaceFormat::setDefaultFormat(format);

                            // 创建离屏表面
                            QOffscreenSurface surface;
                            surface.setFormat(format);
                            surface.create();

                            // 创建 OpenGL 上下文
                            QOpenGLContext context;
                            context.setFormat(format);
                            if (!context.create()) {
                                qWarning() << "Failed to create OpenGL context";
                                return -1;
                            }

                            // 使上下文成为当前上下文
                            if (!context.makeCurrent(&surface)) {
                                qWarning() << "Failed to make OpenGL context current";
                                return -1;
                            }

                            auto jwLayout3d = add_3d_layout(canvas2d, layoutType, plottingWeb, image_spec, availablePaper3D,
                                                            false,
                                                            removeLayerNames3D, removeLayerPrefixes3D);
                            spdlog::info("save project");
                            m_app->saveProject();
                            auto project_dir = m_app->getProjectDir();
                            QString d3_scene_png = QString().append(project_dir).append("/").append("d3_scene.png");
                            spdlog::info("d3 scene png: {}", d3_scene_png.toStdString());
                            jwLayout3d->exportLayoutToImage(layoutType, d3_scene_png);

                            // 释放OpenGL上下文
                            context.doneCurrent();
                        } else {
                            responseDto->error = "enable_3d in config.yaml is false";
                        }
                    } else {
                        spdlog::info("add and export 2d layout");
                        auto appAvailablePapers = m_app->getAvailablePapers();
                        QVector<QString> removeLayerNames = QVector<QString>();
                        QVector<QString> removeLayerPrefixes = QVector<QString>();
                        removeLayerPrefixes.append(REAL3D_TILE_NAME);
                        for (const auto &availablePaper: appAvailablePapers) {
                            spdlog::info("image_spec_name: {}, available_paper: {}", layoutType.toStdString(),
                                         availablePaper.getPaperName().toStdString());
                            auto canvas2d = m_app->getCanvas();
                            add_layout(canvas2d, layoutType, plottingWeb, image_spec, availablePaper, false,
                                       removeLayerNames, removeLayerPrefixes);
                        }
                        spdlog::info("save project");
                        m_app->saveProject();
                        export2DLayout(sceneName, layoutType, plottingWeb, responseDto);
                    }
                }

                /*auto zip_file_name = zipProject(sceneName);
                auto imageSubDir = getImageSubDir(layoutType);
                // 导出图像
                QString paperName = QString::fromStdString(plottingWeb->paper);
                QString project_zip_url = QString(m_mapping_export_nginx_url_prefix)
                        .append("/").append(zip_file_name);
                responseDto->project_zip_url = project_zip_url.toStdString();
                QString imageName = "";
                if (m_export_png_enable) {
                    imageName = exportPNG(sceneName, layoutType, imageSubDir, paperName);
                    QString image_url = QString(m_mapping_export_nginx_url_prefix)
                            .append("/").append(imageSubDir).append("/").append(imageName);
                    responseDto->image_url = image_url.toStdString();
                }
                QString pdfName = "";
                if (m_export_pdf_enable) {
                    pdfName = exportPDF(sceneName, layoutType, imageSubDir, paperName);
                    QString pdf_url = QString(m_mapping_export_nginx_url_prefix)
                            .append("/").append(imageSubDir).append("/").append(imageName);
                    responseDto->pdf_url = pdf_url.toStdString();
                }
                QString svgName = "";
                if (m_export_svg_enable) {
                    svgName = exportSVG(sceneName, layoutType, imageSubDir, paperName);
                    QString svg_url = QString(m_mapping_export_nginx_url_prefix)
                            .append("/").append(imageSubDir).append("/").append(svgName);
                    responseDto->svg_url = svg_url.toStdString();
                }*/
                responseDto->error = "";
                promise->set_value(responseDto);
                spdlog::info("clear layers and project");
                m_app->clearLayers();
                spdlog::info("clean the project");
                m_app->cleanProject();
                spdlog::info("exit invokeMethod lambda");
                eventLoop.quit(); // 退出事件循环
            } catch (const std::exception &e) {
                spdlog::error("Exception in invokeMethod lambda: {}", e.what());
                promise->set_exception(std::make_exception_ptr(e));
                responseDto->error = e.what();
                eventLoop.quit(); // 退出事件循环
            }
            return 0;
        }, Qt::QueuedConnection);
        // 启动事件循环，直到 lambda 执行完成
        eventLoop.exec();
        return promise->get_future().get();
    });
}


void Processor::export2DLayout(QString& sceneName,
                               const QString& layoutType,
                               const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingWeb,
                               DTOWRAPPERNS::DTOWrapper<ResponseDto> responseDto) {
    auto zip_file_name = zipProject(sceneName);
    auto imageSubDir = getImageSubDir(layoutType);
    // 导出图像
    QString paperName = QString::fromStdString(plottingWeb->paper);
    QString project_zip_url = QString(m_mapping_export_nginx_url_prefix)
            .append("/").append(zip_file_name);
    responseDto->project_zip_url = project_zip_url.toStdString();
    QString imageName = "";
    if (m_export_png_enable) {
        imageName = exportPNG(sceneName, layoutType, imageSubDir, paperName);
        QString image_url = QString(m_mapping_export_nginx_url_prefix)
                .append("/").append(imageSubDir).append("/").append(imageName);
        responseDto->image_url = image_url.toStdString();
    }
    QString pdfName = "";
    if (m_export_pdf_enable) {
        pdfName = exportPDF(sceneName, layoutType, imageSubDir, paperName);
        QString pdf_url = QString(m_mapping_export_nginx_url_prefix)
                .append("/").append(imageSubDir).append("/").append(imageName);
        responseDto->pdf_url = pdf_url.toStdString();
    }
    QString svgName = "";
    if (m_export_svg_enable) {
        svgName = exportSVG(sceneName, layoutType, imageSubDir, paperName);
        QString svg_url = QString(m_mapping_export_nginx_url_prefix)
                .append("/").append(imageSubDir).append("/").append(svgName);
        responseDto->svg_url = svg_url.toStdString();
    }
}


void Processor::checkDealWithClosedGeometry(const DTOWRAPPERNS::DTOWrapper<GeoPolygonJsonDto> &geojson) {
    // 检查geojson是否包含有效的Polygon数据
    if (geojson->geometry && geojson->geometry->type == "Polygon" && geojson->geometry->coordinates) {
        if (!geojson || !(geojson->geometry) || !geojson->geometry->coordinates
            || !geojson->geometry->coordinates[0]
            || geojson->geometry->coordinates[0]->size() < 4) {
            throw GeometryCheckError("Invalid Polygon data");
        }

        /*auto isEqX = DOUBLECOMPARENEAR(geojson->geometry->coordinates[0][0][0], geojson->geometry->coordinates[0][geojson->geometry->coordinates[0]->size() - 1][0]);
        auto isEqY = DOUBLECOMPARENEAR(geojson->geometry->coordinates[0][0][1], geojson->geometry->coordinates[0][geojson->geometry->coordinates[0]->size() - 1][1]);
        spdlog::info("isEqX: {}", isEqX);
        spdlog::info("isEqY: {}", isEqY);*/

        auto isEq = POINTXYCOMPARENEAR(geojson->geometry->coordinates[0][0],
                                       geojson->geometry->coordinates[0][geojson->geometry->coordinates[0]->size() -
                                                                         1]);
        //if (!isEqX || !isEqY ) {
        if (!isEq) {
            // 处理闭合几何图形
            geojson->geometry->coordinates[0]->push_back(geojson->geometry->coordinates[0][0]);
        } else {
//            spdlog::info("first point x: {}, y: {}", geojson->geometry->coordinates[0][0][0],
//                         geojson->geometry->coordinates[0][0][1]);
//            spdlog::info("last point x: {}, y: {}",
//                         geojson->geometry->coordinates[0][geojson->geometry->coordinates[0]->size() - 1][0],
//                         geojson->geometry->coordinates[0][geojson->geometry->coordinates[0]->size() - 1][1]);
        }
    }
}


void Processor::plottingLayers(const DTOWRAPPERNS::DTOWrapper<PlottingRespDto> &plotting_data) {
    auto map_plot_payloads = plotting_data->data;

    // 遍历每个 payload
    for (const auto &payloads: *map_plot_payloads) {

        auto plottings = payloads->plottings;
        QList<QString> name_list;
        QList<QJsonObject> style_list;
        QList<QJsonObject> shape_list;
        QJsonObject layer_style = payloads->getLayerStyleJson();
        QJsonObject font_style = payloads->getFontStyleJson();
        for (const auto &plotting: *plottings) {
            auto name = plotting->name;
            name_list.append(QString::fromStdString(name));
            style_list.append(plotting->getStyleInfoJson());
            shape_list.append(plotting->getShapeJson());
        }

//        spdlog::info("plotting pcode:{} code:{}", payloads->pcode->c_str(), payloads->code->c_str());

        auto shapeType = layer_style.contains("shapeType") ? layer_style["shapeType"].toString() : "";

        if (shapeType == "04") {
            // 等级域 特殊处理 同心圆
            QList<QList<double>> circle_geometry_coordinates_list;
            QList<int> polygon_geometry_properties_radius;
            for (const auto &shape: shape_list) {
                if (shape.contains("geometry")) {
                    auto geometry = shape["geometry"].toObject();
                    if (geometry.contains("type")) {
                        auto type = geometry["type"].toString();
                        if (type == "Point") {
                            auto coordinates = geometry["coordinates"].toArray();
                            auto coord = coordinates[0].toDouble();
                            QList<double> circle_geometry_coordinates = {coordinates[0].toDouble(),
                                                                         coordinates[1].toDouble(),
                                                                         coordinates[2].toDouble()};
                            circle_geometry_coordinates_list.append(circle_geometry_coordinates);
                        }
                    }
                }
                if (shape.contains("properties")) {
                    auto properties = shape["properties"].toObject();
                    if (properties.contains("radius")) {
                        auto radius = properties["radius"].toInt();
                        polygon_geometry_properties_radius.append(radius);
                    }
                }
            }
            qDebug() << "circle_geometry_coordinates_list: " << circle_geometry_coordinates_list;
            qDebug() << "polygon_geometry_properties_radius: " << polygon_geometry_properties_radius;

            QList<QList<double>> style_percents;
            QList<QList<QString>> style_color_list;
            QList<QList<double>> style_color_opacity_list;
            for (const auto &item: style_list) {
                auto style = item;
                if (style.contains("layerStyleObj")) {
                    auto layerStyleObj = style["layerStyleObj"].toObject();
                    if (layerStyleObj.contains("djy")) {
                        auto djy = layerStyleObj["djy"].toArray();
                        QList<double> djyPercentList;
                        QList<QString> djyColorList;
                        QList<double> djyOpacityList;
                        for (const auto &itemDjy: djy) {
                            auto djyItem = itemDjy.toObject();
                            if (djyItem.contains("num")) {
                                auto percent = djyItem["num"].toDouble();
                                djyPercentList.append(percent);
                            }
                            if (djyItem.contains("color")) {
                                auto color = djyItem["color"].toString();
                                auto qColor = ColorTransformUtil::strRgbaToHex(color);
                                djyColorList.append(qColor.first);
                                djyOpacityList.append(qColor.second);
                            }
                        }
                        style_percents.append(djyPercentList);
                        style_color_list.append(djyColorList);
                        style_color_opacity_list.append(djyOpacityList);
                    }
                }
            }
            qDebug() << "style_percents: " << style_percents;
            qDebug() << "style_color_list: " << style_color_list;

            auto grouped_color = ColorTransformUtil::multiColorGroup(style_color_list);

            auto style_grouped = _grouped_circle_by_color_grouped(
                    grouped_color,
                    circle_geometry_coordinates_list,
                    polygon_geometry_properties_radius,
                    style_percents,
                    style_color_list,
                    style_color_opacity_list);

            if (m_verbose) {
                qDebug() << "style_grouped: " << style_grouped;
                qDebug() << "style_grouped json: "
                         << JsonUtil::variantMapToJson(style_grouped).toJson(QJsonDocument::JsonFormat::Compact);
            }

            int circle_num = 0;
            QVariantMap::iterator it;
            for (it = style_grouped.begin(); it != style_grouped.end(); ++it) {
                const auto &color_style = it.key();
                auto color_style_dict = style_grouped.value(color_style).toMap();
                qDebug() << "color_style: " << color_style << " --> " << color_style_dict;
                QString layerPrefix = QString::fromStdString(payloads->name);
                QString layerName = QString("%1%2").arg(layerPrefix, QString::number(circle_num));
                auto jw_circle = std::make_unique<JwCircle>(
                        m_app->getSceneName(),
                        layerName,
                        m_app->getProjectDir(),
                        m_app->getProject(),
                        m_app->getTransformContext()
                );

                QList<QgsPoint> pointsList;
                if (m_verbose) {
                    qDebug() << "polygon_geometry_coordinates_list: "
                             << color_style_dict["polygon_geometry_coordinates_list"];
                }
                auto coordPointsList = color_style_dict["polygon_geometry_coordinates_list"].toList();
                for (const auto &coordPoint: coordPointsList) {
                    auto coordPointList = coordPoint.toList();
                    pointsList.append(QgsPoint(coordPointList[0].toDouble(), coordPointList[1].toDouble(),
                                               coordPointList[2].toDouble()));
                }
                if (m_verbose) {
                    qDebug() << "polygon_geometry_properties_radius: "
                             << color_style_dict["polygon_geometry_properties_radius"];
                }
                auto radiusQVariants = color_style_dict["polygon_geometry_properties_radius"].toList();
                QList<double> radiusDoubleList;
                for (const auto &radiusQVariant: radiusQVariants) {
                    if (radiusQVariant.canConvert<double>()) {
                        radiusDoubleList.append(radiusQVariant.toDouble());
                    }
                }

                QList<QColor> areasColorList;
                if (m_verbose) {
                    qDebug() << "areas_color_list: " << color_style_dict["areas_color_list"];
                }
                auto areas_color_list = color_style_dict["areas_color_list"].toList();
                for (const auto &color: areas_color_list) {
                    if (color.canConvert<QString>()) {
                        areasColorList.append(QColor(color.toString()));
                    }
                }
                // todo throw exception when areasColorList is empty
                QList<float> styleColorOpacityList;
                if (m_verbose) {
                    qDebug() << "areas_opacity_list: " << color_style_dict["areas_opacity_list"];
                }
                auto areas_opacity_list = color_style_dict["areas_opacity_list"].toList();
                for (const auto &item: areas_opacity_list) {
                    if (item.canConvert<double>()) {
                        styleColorOpacityList.append(item.toFloat());
                    }
                }
                // add level key areas
                if (m_verbose) {
                    for (const auto &item: pointsList) {
                        qDebug() << "add level key areas -> pointsList: " << item.x() << ", " << item.y() << ", "
                                 << item.z();
                    }
                    qDebug() << "add level key areas -> radiusDoubleList size: " << radiusDoubleList.size();
                    for (const auto &item: radiusDoubleList) {
                        qDebug() << "add level key areas -> radiusDoubleList: " << item;
                    }

                    qDebug() << "add level key areas -> style_percents size:" << style_percents.size();
                    for (const auto &item: style_percents) {
                        qDebug() << "add level key areas -> style_percents size: " << item.size();
                        for (const auto &subItem: item) {
                            qDebug() << "add level key areas -> style_percents: " << subItem;
                        }
                    }

                    qDebug() << "add level key areas -> areasColorList size:" << areasColorList.size();
                    for (const auto &item: areasColorList) {
                        qDebug() << "add level key areas -> areasColorList: " << item;
                    }

                    qDebug() << "add level key areas -> styleColorOpacityList size:" << styleColorOpacityList.size();
                    for (const auto &item: styleColorOpacityList) {
                        qDebug() << "add level key areas -> styleColorOpacityList: " << item;
                    }
                }

                jw_circle->addLevelKeyAreas(
                        pointsList,
                        radiusDoubleList,
                        style_percents,
                        areasColorList,
                        styleColorOpacityList,
                        72
                );
                circle_num++;
            }

        } else {
            // others, not 等级域
            QList<QList<double>> point_geometry_coordinates_list;  // points list
            QList<QList<QList<double>>> line_geometry_coordinates_list; // lineString list
            QList<QList<QList<QList<double >> >> polygon_geometry_coordinates_list; // polygon list
            QList<double> circle_radii; // circle radius list

            for (const auto &shape: shape_list) {
                if (shape.contains("properties")) {
                    auto properties = shape["properties"].toObject();
                    if (properties.contains("subType")) {
                        auto subType = properties["subType"].toString();
                        if (subType == "Circle") {
                            if (properties.contains("radius")) {
                                auto radius = properties["radius"].toDouble();
                                circle_radii.append(radius);
                            }
                        }
                    }
                }

                if (shape.contains("geometry")) {
                    auto geometry = shape["geometry"].toObject();
                    if (geometry.contains("type")) {
                        auto type = geometry["type"].toString();
                        if (type == "Point") {
                            auto coordinates = geometry["coordinates"].toArray();
                            auto coord = coordinates[0].toDouble();
                            QList<double> point_geometry_coordinates = {coordinates[0].toDouble(),
                                                                        coordinates[1].toDouble(),
                                                                        coordinates[2].toDouble()};
                            point_geometry_coordinates_list.append(point_geometry_coordinates);
                        }
                        if (type == "LineString") {
                            auto coordinates = geometry["coordinates"].toArray();
                            QList<QList<double>> line_geometry_coordinates;
                            for (const auto &coord: coordinates) {
                                auto coordList = coord.toArray();
                                QList<double> line_geometry_coordinate = {coordList[0].toDouble(),
                                                                          coordList[1].toDouble(),
                                                                          coordList[2].toDouble()};
                                line_geometry_coordinates.append(line_geometry_coordinate);
                            }
                            line_geometry_coordinates_list.append(line_geometry_coordinates);
                        }
                        if (type == "Polygon") {
                            auto coordinates = geometry["coordinates"].toArray();
                            QList<QList<QList<double>>> polygon_geometry_coordinates;
                            for (const auto &coordArr: coordinates) {
                                QList<QList<double>> line_polygon_geometry_coordinate;
                                auto lineCoordArray = coordArr.toArray();
                                for (const auto &coordList: lineCoordArray) {
                                    auto pointCoordArray = coordList.toArray();
                                    QList<double> point_polygon_geometry_coordinate_list = {
                                            pointCoordArray[0].toDouble(),
                                            pointCoordArray[1].toDouble(),
                                            pointCoordArray[2].toDouble()};
                                    line_polygon_geometry_coordinate.append(point_polygon_geometry_coordinate_list);
                                }
                                polygon_geometry_coordinates.append(line_polygon_geometry_coordinate);
                            }
                            polygon_geometry_coordinates_list.append(polygon_geometry_coordinates);
                        }
                    }
                }
            }

            // paint the circles and points to layer
            if (!point_geometry_coordinates_list.empty()) {
                QString layerName = QString::fromStdString(payloads->name);
                if (!circle_radii.empty()) {
                    auto jw_circle = std::make_unique<JwCircle>(
                            m_app->getSceneName(),
                            layerName,
                            m_app->getProjectDir(),
                            m_app->getProject(),
                            m_app->getTransformContext()
                    );
                    QString iconName = QString::fromStdString(payloads->name);
                    QList<QgsPoint> pointsList;
                    for (const auto &coordPoint: point_geometry_coordinates_list) {
                        pointsList.append(QgsPoint(coordPoint[0], coordPoint[1], coordPoint[2]));
                    }
                    jw_circle->addCircles(iconName, name_list, pointsList, circle_radii, layer_style, style_list);
                } else {
                    auto jw_point = std::make_unique<JwPoint>(
                            m_app->getSceneName(),
                            layerName,
                            m_app->getProjectDir(),
                            m_app->getProject(),
                            m_app->getTransformContext()
                    );
                    QString iconName = QString::fromStdString(payloads->name);
                    QList<QgsPoint> pointsList;
                    for (const auto &coordPoint: point_geometry_coordinates_list) {
                        pointsList.append(QgsPoint(coordPoint[0], coordPoint[1], coordPoint[2]));
                    }
                    QString attachment = QString::fromStdString(payloads->attachment);
                    jw_point->addPoints(iconName, name_list, pointsList, font_style, layer_style, style_list, 20,
                                        attachment);
                }
            }
            // paint the lineString to layer
            if (!line_geometry_coordinates_list.empty()) {
                auto grouped_color = _grouped_color_lines(
                        name_list,
                        line_geometry_coordinates_list,
                        style_list
                );
                int line_num = 0;
                QVariantMap::iterator it;
                //for (const auto &color_style: grouped_color.keys()) {
                for (it = grouped_color.begin(); it != grouped_color.end(); ++it) {
                    const auto &color_style = it.key();
                    auto color_style_dict = grouped_color.value(color_style).toMap();
                    QString layerPrefix = QString::fromStdString(payloads->name);
                    QString layerName = QString("%1%2").arg(layerPrefix, QString::number(line_num));
                    auto jw_line = std::make_unique<JwLine>(
                            m_app->getSceneName(),
                            layerName,
                            m_app->getProjectDir(),
                            m_app->getProject(),
                            m_app->getTransformContext()
                    );

                    QList<QString> lineNameList;
                    auto line_name_list = color_style_dict["name_list"].toList();
                    for (const auto &name: line_name_list) {
                        if (name.canConvert<QString>()) {
                            lineNameList.append(name.toString());
                        }
                    }
                    QList<QgsLineString> linesGeometryCoordinates;
                    auto geometry_coordinates_list = color_style_dict["geometry_coordinates_list"].toList();
                    auto typed_geometry_coordinates_list = TypeConvert::convertVariant<QList<QList<QList<double>>>>(
                            geometry_coordinates_list);
                    auto coordinate_list_json = JsonUtil::convertQListNest3ToJson(typed_geometry_coordinates_list);
                    if (m_verbose) {
                        qDebug() << "coordinate_list_json: "
                                 << coordinate_list_json.toJson(QJsonDocument::JsonFormat::Compact);
                    }
                    for (const auto &coordLines: typed_geometry_coordinates_list) {
                        QgsLineString line_geometry_coordinate;
                        auto coordPoints = coordLines;
                        for (const auto &coordPoint: coordPoints) {
                            auto coordPointList = coordPoint;
                            QgsPoint line_vertexes = QgsPoint(coordPointList[0],
                                                              coordPointList[1],
                                                              coordPointList[2]);
                            line_geometry_coordinate.addVertex(line_vertexes);
                        }
                        linesGeometryCoordinates.append(line_geometry_coordinate);
                    }
                    if (m_verbose) {
                        qDebug() << "linesGeometryCoordinates size: " << linesGeometryCoordinates.size();
                        for (const auto &line: linesGeometryCoordinates) {
                            qDebug() << "linesGeometryCoordinates: "
                                     << QString::fromStdString(ShowDataUtil::lineStringToString(line));
                        }
                    }
                    QList<QJsonObject> styleList;
                    auto style_list_ = color_style_dict["style_list"].toList();
                    for (const auto &item: style_list_) {
                        if (item.canConvert<QJsonObject>()) {
                            styleList.append(item.value<QJsonObject>());
                        }
                    }
                    jw_line->addLines(
                            lineNameList,
                            linesGeometryCoordinates,
                            font_style,
                            layer_style,
                            styleList
                    );
                    line_num++;
                }
            }
            // paint the polygon to layer
            if (!polygon_geometry_coordinates_list.empty()) {
                auto grouped_color = _grouped_color_polygons(
                        name_list,
                        polygon_geometry_coordinates_list,
                        style_list
                );
                int polygon_num = 0;
                QVariantMap::iterator it;
                //for (const auto &color_style: grouped_color.keys()) {
                for (it = grouped_color.begin(); it != grouped_color.end(); ++it) {
                    const auto &color_style = it.key();
                    auto color_style_dict = grouped_color.value(color_style).toMap();
                    QString layerPrefix = QString::fromStdString(payloads->name);
                    QString layerName = QString("%1%2").arg(layerPrefix, QString::number(polygon_num));
                    auto jw_polygon = std::make_unique<JwPolygon>(
                            m_app->getSceneName(),
                            layerName,
                            m_app->getProjectDir(),
                            m_app->getProject(),
                            m_app->getTransformContext()
                    );

                    QList<QString> polygonNameList;
                    auto polygon_name_list = color_style_dict["name_list"].toList();
                    for (const auto &name: polygon_name_list) {
                        if (name.canConvert<QString>()) {
                            polygonNameList.append(name.toString());
                        }
                    }
                    QList<QgsPolygon> polygonGeometryCoordinates;
                    auto geometry_coordinates_list = color_style_dict["geometry_coordinates_list"].toList();
                    auto typed_geometry_coordinates_list = TypeConvert::convertVariant<QList<QList<QList<QList<double>>>>>(
                            geometry_coordinates_list);
                    for (const auto &coordPolygons: typed_geometry_coordinates_list) {
                        QgsLineString lineString;
                        for (const auto &coordLines: coordPolygons) {
                            for (const auto &coordPoint: coordLines) {
                                QgsPoint point_vertex_of_polygon = QgsPoint(coordPoint[0],
                                                                            coordPoint[1],
                                                                            coordPoint[2]);
                                lineString.addVertex(point_vertex_of_polygon);
                            }
                        }
                        QgsPolygon qgsPolygon;
                        Formula::checkAndClosedLineStringOfPolygon(lineString);
                        qgsPolygon.setExteriorRing(lineString.clone());
                        polygonGeometryCoordinates.append(qgsPolygon);
                    }
                    if (m_verbose) {
                        qDebug() << "polygonGeometryCoordinates size: " << polygonGeometryCoordinates.size();
                        for (const auto &polygon: polygonGeometryCoordinates) {
                            qDebug() << "polygonGeometryCoordinates: "
                                     << QString::fromStdString(ShowDataUtil::polygonToString(polygon));
                        }
                    }
                    QList<QJsonObject> styleList;
                    auto style_list_ = color_style_dict["style_list"].toList();
                    for (const auto &item: style_list_) {
                        if (item.canConvert<QJsonObject>()) {
                            styleList.append(item.value<QJsonObject>());
                        }
                    }
                    jw_polygon->addPolygons(
                            polygonNameList,
                            polygonGeometryCoordinates,
                            font_style,
                            layer_style,
                            styleList
                    );
                    polygon_num++;
                }
            }
        }
    }
}


// 添加2d布局
void Processor::add_layout(
        QgsMapCanvas *canvas,
        const QString &layout_name,
        const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingWeb,
        const QMap<QString, QVariant> &image_spec,
        const PaperSpecification &available_paper,
        bool write_qpt,
        const QVector<QString> &removeLayerNames,
        const QVector<QString> &removeLayerPrefixs) {
    auto joinedLayoutName = layout_name + "-" + available_paper.getPaperName();
    spdlog::info("add layout: {}", joinedLayoutName.toStdString());

    auto project = m_app->getProject();
    auto sceneName = m_app->getSceneName();
    auto projectDir = m_app->getProjectDir();
    JwLayout jwLayout(project, canvas, sceneName, image_spec, projectDir, joinedLayoutName);

    auto plottingWebJsonDoc = JsonUtil::convertDtoToQJsonObject(plottingWeb);
    auto plottingWebMap = JsonUtil::jsonObjectToVariantMap(plottingWebJsonDoc.object());

    jwLayout.addPrintLayout(QString("2d"), joinedLayoutName, plottingWebMap, available_paper, write_qpt,
                            removeLayerNames, removeLayerPrefixs);
    spdlog::debug("add_2d_layout done");
}

// 添加3d布局
JwLayout3D* Processor::add_3d_layout(
        QgsMapCanvas *canvas,
        const QString &layout_name,
        const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingWeb,
        const QMap<QString, QVariant> &image_spec,
        const PaperSpecification &available_paper,
        bool write_qpt,
        const QVector<QString> &removeLayerNames,
        const QVector<QString> &removeLayerPrefixes) {
    auto joinedLayoutName = QString().append(layout_name).append("-").append(available_paper.getPaperName()).append(
            "-3D");
    spdlog::info("add layout: {}", joinedLayoutName.toStdString());

    auto canvas3d = std::make_unique<Qgs3DMapCanvas>();
    auto project = m_app->getProject();
    auto sceneName = m_app->getSceneName();
    auto projectDir = m_app->getProjectDir();
    /*JwLayout3D jwLayout3d(project, canvas, canvas3d.get(),
                          sceneName, image_spec, projectDir, joinedLayoutName);*/
    auto jwLayout3d = std::make_unique<JwLayout3D>(project, canvas, canvas3d.get(),
                                                   sceneName, image_spec, projectDir, joinedLayoutName);

    auto plottingWebJsonDoc = JsonUtil::convertDtoToQJsonObject(plottingWeb);
    auto plottingWebMap = JsonUtil::jsonObjectToVariantMap(plottingWebJsonDoc.object());

    jwLayout3d->init3DMapSettings(removeLayerNames, removeLayerPrefixes);
    jwLayout3d->set3DCanvas();
    jwLayout3d->addPrintLayout(QString("3d"), joinedLayoutName, plottingWebMap, available_paper, write_qpt);
    return jwLayout3d.release();
}

QString Processor::zipProject(const QString &scene_name) {
    QString targetZipFile = QString("%1/%2.zip").arg(m_export_prefix, scene_name);
    spdlog::info("zip project: {}", targetZipFile.toStdString());
    CompressUtil::create_zip(m_app->getProjectDir().toStdString(), targetZipFile.toStdString());
    QString zip_file_name = QString().append(scene_name).append(".zip");
    return zip_file_name;
}

QString Processor::getImageSubDir(const QString &layout_name) {
    auto image_spec = m_setting_image_spec->value(layout_name).toMap();
    if (image_spec.isEmpty()) {
        return "";
    }
    if (!image_spec.contains("local")) {
        return "";
    }
    return image_spec["local"].toString();
}

QString Processor::exportPNG(const QString &sceneName, const QString &layoutName, const QString &imageSubDir,
                             const QString &paperName) {
    QString imageName = QString("%1-%2-%3.png").arg(sceneName, layoutName, paperName);
    QString outputPath = QString("%1/%2/%3").arg(m_export_prefix, imageSubDir, imageName);
    FileUtil::delete_file(outputPath);
    spdlog::info("export image -> outputPath: {}", outputPath.toStdString());
    auto isExportStatus = m_app->exportLayoutAsPng(layoutName, outputPath, paperName);
    spdlog::info("export status: {}", isExportStatus);
    return imageName;
}

QString Processor::exportPDF(const QString &sceneName, const QString &layoutName, const QString &imageSubDir,
                             const QString &paperName) {
    QString imageName = QString("%1-%2-%3.pdf").arg(sceneName, layoutName, paperName);
    QString outputPath = QString("%1/%2/%3").arg(m_export_prefix, imageSubDir, imageName);
    FileUtil::delete_file(outputPath);
    spdlog::info("export image -> outputPath: {}", outputPath.toStdString());
    auto isExportStatus = m_app->exportLayoutAsPdf(layoutName, outputPath, paperName);
    spdlog::info("export status: {}", isExportStatus);
    return imageName;
}

QString Processor::exportSVG(const QString &sceneName, const QString &layoutName, const QString &imageSubDir,
                             const QString &paperName) {
    QString imageName = QString("%1-%2-%3.svg").arg(sceneName, layoutName, paperName);
    QString outputPath = QString("%1/%2/%3").arg(m_export_prefix, imageSubDir, imageName);
    FileUtil::delete_file(outputPath);
    spdlog::info("export image -> outputPath: {}", outputPath.toStdString());
    auto isExportStatus = m_app->exportLayoutAsSvg(layoutName, outputPath, paperName);
    spdlog::info("export status: {}", isExportStatus);
    return imageName;
}

QVariantMap Processor::_grouped_circle_by_color_grouped(
        QMap<QString, int> &grouped_color,
        QList<QList<double>> &polygon_geometry_coordinates_list,
        QList<int> &polygon_geometry_properties_radius,
        QList<QList<double>> &style_percents,
        QList<QList<QString>> &areas_color_list,
        QList<QList<double>> &areas_opacity_list
) {
    QVariantMap style_grouped;
    for (int i = 0; i < areas_color_list.size(); ++i) {
        auto colors = areas_color_list[i];
        QString merged_areas_color = ColorTransformUtil::mergeColor(colors);
        if (style_grouped.contains(merged_areas_color)) {
            auto mergedAreaColorDict = style_grouped.value(merged_areas_color);
            auto mergedMap = mergedAreaColorDict.toMap();

            // 更新 polygon_geometry_coordinates_list
            QVariantList coordinatesList = mergedMap["polygon_geometry_coordinates_list"].toList();
            QVariantList nestedList;
            for (const auto &coord: polygon_geometry_coordinates_list[i]) {
                nestedList.append(coord);
            }
            coordinatesList.insert(coordinatesList.size(), QVariant(nestedList));
            mergedMap["polygon_geometry_coordinates_list"] = coordinatesList;

            // 更新 polygon_geometry_properties_radius
            QVariantList radiusList = mergedMap["polygon_geometry_properties_radius"].toList();
            radiusList.append(polygon_geometry_properties_radius[i]);
            mergedMap["polygon_geometry_properties_radius"] = radiusList;

            // 更新 style_percents
            QVariantList percentsList = mergedMap["style_percents"].toList();
            QVariantList percents;
            for (const auto &percent: style_percents[i]) {
                percents.append(percent);
            }
            percentsList.insert(percentsList.size(), QVariant(percents));
            mergedMap["style_percents"] = percentsList;

            // 更新 areas_color_list
            QVariantList colorList = mergedMap["areas_color_list"].toList();
            for (const auto &color: areas_color_list[i]) {
                if (!colorList.contains(color)) {
                    colorList.append(color);
                }
            }
            mergedMap["areas_color_list"] = colorList;

            // 更新 areas_opacity_list
            QVariantList opacityList = mergedMap["areas_opacity_list"].toList();
            for (const auto &opacity: areas_opacity_list[i]) {
                if (!opacityList.contains(opacity)) {
                    opacityList.append(opacity);
                }
            }
            mergedMap["areas_opacity_list"] = opacityList;
            style_grouped.insert(merged_areas_color, mergedMap);
        } else {
            QVariantMap data;

            QVariantList geometryList;
            QVariantList coordinatesList;
            for (const auto &coord: polygon_geometry_coordinates_list[i]) {
                coordinatesList.append(coord);
            }
            geometryList.insert(geometryList.size(), QVariant(coordinatesList));
            data.insert("polygon_geometry_coordinates_list", geometryList);

            QVariantList radiusList;
            radiusList.append(polygon_geometry_properties_radius[i]);
            data.insert("polygon_geometry_properties_radius", radiusList);

            QVariantList stylePercentsList;
            QVariantList percentsList;
            for (const auto &percent: style_percents[i]) {
                percentsList.append(percent);
            }
            stylePercentsList.insert(stylePercentsList.size(), QVariant(percentsList));
            data.insert("style_percents", stylePercentsList);

            QVariantList areasColorList;
            for (const auto &color: areas_color_list[i]) {
                areasColorList.append(color);
            }
            data.insert("areas_color_list", areasColorList);

            QVariantList areasOpacityList;
            for (const auto &opacity: areas_opacity_list[i]) {
                areasOpacityList.append(opacity);
            }
            data.insert("areas_opacity_list", areasOpacityList);

            style_grouped.insert(merged_areas_color, data);
        }
    }
    return style_grouped;
}

QVariantMap Processor::_grouped_color_lines(
        QList<QString> &name_list,
        QList<QList<QList<double>>> &geometry_coordinates_list,
        QList<QJsonObject> &style_list) {
    QVariantMap color_dict;
    for (int i = 0; i < style_list.size(); ++i) {
        QString style_color;
        auto style = style_list[i];
        if (style.contains("layerStyle") && style["layerStyle"].isObject()) {
            auto layerStyle = style["layerStyle"].toObject();
            if (layerStyle.contains("color")) {
                //style_color = layerStyle["color"].toString();
                style_color = ColorTransformUtil::strRgbaToHex(layerStyle["color"].toString()).first;
            }
        }
        if (style_color.isEmpty()) {
            //style_color = "#000000"; // 默认颜色
            style_color = ColorTransformUtil::strRgbaToHex("rgba(0, 153, 68, 1)").first;
        }

        if (color_dict.contains(style_color)) {
            auto colorDict = color_dict.value(style_color);
            auto colorMap = colorDict.toMap();

            // 修改 name_list 的处理方式，保证是一个列表
            QVariantList nameList = colorMap["name_list"].toList();
            nameList.append(name_list[i]);
            colorMap["name_list"] = nameList;

            // 修改 geometry_coordinates_list 的处理方式，保证是一个列表
            QVariantList geometryList = colorMap["geometry_coordinates_list"].toList();
            QVariantList lineList;
            qDebug() << "geometry_coordinates_list[i]: " << geometry_coordinates_list[i];
            for (const auto &coordsLine: geometry_coordinates_list[i]) {
                QVariantList pointList;
                for (const auto &coord: coordsLine) {
                    pointList.append(coord);
                    qDebug() << "pointList: " << pointList;
                }
                lineList.insert(lineList.size(), QVariant(pointList));
                qDebug() << "lineList: " << lineList;
            }
            geometryList.insert(geometryList.size(), QVariant(lineList));
            colorMap["geometry_coordinates_list"] = geometryList;

            // 修改 style_list 的处理方式，保证是一个列表
            QVariantList styleList = colorMap["style_list"].toList();
            styleList.append(style_list[i]);
            colorMap["style_list"] = styleList;

            color_dict.insert(style_color, colorMap);
        } else {
            QVariantMap data;
            QVariantList nameList;
            nameList.append(name_list[i]);
            data.insert("name_list", nameList);

            QVariantList geometryList;
            QVariantList lineList;
            qDebug() << "geometry_coordinates_list[i]: " << geometry_coordinates_list[i];
            for (const auto &coordsLine: geometry_coordinates_list[i]) {
                QVariantList pointList;
                for (const auto &coord: coordsLine) {
                    pointList.append(coord);
                    qDebug() << "pointList: " << pointList;
                }
                lineList.insert(lineList.size(), QVariant(pointList));
                qDebug() << "lineList: " << lineList;
            }
            geometryList.insert(geometryList.size(), QVariant(lineList));
            qDebug() << "geometryList: " << geometryList;
            data.insert("geometry_coordinates_list", geometryList);

            QVariantList styleList;
            styleList.append(style_list[i]);
            data.insert("style_list", styleList);

            color_dict.insert(style_color, data);
        }
    }
    return color_dict;
}

QVariantMap Processor::_grouped_color_polygons(
        QList<QString> &name_list,
        QList<QList<QList<QList<double>>>> &geometry_coordinates_list,
        QList<QJsonObject> &style_list) {
    QVariantMap color_dict;
    for (int i = 0; i < style_list.size(); ++i) {
        QString style_color;
        auto style = style_list[i];
        if (style.contains("layerStyle") && style["layerStyle"].isObject()) {
            auto layerStyle = style["layerStyle"].toObject();
            if (layerStyle.contains("color")) {
                style_color = ColorTransformUtil::strRgbaToHex(layerStyle["color"].toString()).first;
            }
        }
        if (style_color.isEmpty()) {
            style_color = ColorTransformUtil::strRgbaToHex("rgba(0, 153, 68, 1)").first;
        }

        if (color_dict.contains(style_color)) {
            auto colorDict = color_dict.value(style_color);
            auto colorMap = colorDict.toMap();

            // 修改 name_list 的处理方式，保证是一个列表
            QVariantList nameList = colorMap["name_list"].toList();
            nameList.append(name_list[i]);
            colorMap["name_list"] = nameList;

            // 修改 geometry_coordinates_list 的处理方式，保证是一个列表
            QVariantList geometryList = colorMap["geometry_coordinates_list"].toList();
            QVariantList polygonList;
            for (const auto &coordLineList: geometry_coordinates_list[i]) {
                QVariantList lineList;
                for (const auto &pointCoordList: coordLineList) {
                    QVariantList pointList;
                    for (const auto &coord: pointCoordList) {
                        pointList.append(coord);
                    }
                    lineList.insert(lineList.size(), QVariant(pointList));
                }
                polygonList.insert(polygonList.size(), QVariant(lineList));
            }
            geometryList.insert(geometryList.size(), QVariant(polygonList));
            colorMap["geometry_coordinates_list"] = geometryList;

            // 修改 style_list 的处理方式，保证是一个列表
            QVariantList styleList = colorMap["style_list"].toList();
            styleList.append(style_list[i]);
            colorMap["style_list"] = styleList;

            color_dict.insert(style_color, colorMap);
        } else {
            QVariantMap data;
            QVariantList nameList;
            nameList.append(name_list[i]);
            data.insert("name_list", nameList);

            QVariantList geometryList;
            QVariantList polygonList;
            for (const auto &coordLineList: geometry_coordinates_list[i]) {
                QVariantList lineList;
                for (const auto &pointCoordList: coordLineList) {
                    QVariantList pointList;
                    for (const auto &coord: pointCoordList) {
                        pointList.append(coord);
                    }
                    lineList.insert(lineList.size(), QVariant(pointList));
                }
                polygonList.insert(polygonList.size(), QVariant(lineList));
            }
            geometryList.insert(geometryList.size(), QVariant(polygonList));
            data.insert("geometry_coordinates_list", geometryList);

            QVariantList styleList;
            styleList.append(style_list[i]);
            data.insert("style_list", styleList);

            color_dict.insert(style_color, data);
        }
    }
    return color_dict;
}
