//
// Created by etl on 2/10/25.
//

#include "qcorestarter.h"


//
// Created by etl on 24-12-18.
//

QCoreStarter::QCoreStarter() {}

QCoreStarter::~QCoreStarter() {
    /*if (mQSurfaceFormat) {
        mQSurfaceFormat.reset();
    }
    if (mQOffscreenSurface) {
        mQOffscreenSurface.reset();
    }
    if (mOpenGLContext) {
        mOpenGLContext.reset();
    }*/
    if (mQgsApp) {
        mQgsApp.reset();
    }
    if (mApp) {
        mApp.reset();
    }
}

BaseStarter* QCoreStarter::GetInstance() {
    return this;
}

void QCoreStarter::Init(StarterContext& context) {
//    spdlog::info("QCoreStarter Init start");
    auto config = context.Props();
    int newArgc;
    //char** newArgv;
    std::unique_ptr<char*[]> newArgv;
    context.getConvertedArgs(newArgc, newArgv);

    // 确保 newArgv 的内存有效性
    std::vector<std::string> argsStorage(newArgc);
    std::vector<char*> argsPtrs(newArgc);
    for (int i = 0; i < newArgc; ++i) {
        argsStorage[i] = newArgv[i];  // 复制字符串
        argsPtrs[i] = &argsStorage[i][0];  // 获取 C 风格字符串指针
    }

    bool GUIenabled = false;
    try {
        GUIenabled = (*config)["qgis"]["gui_enabled"].as<bool>();
//        spdlog::info("GUIenabled: {}", GUIenabled);
    } catch (const std::exception& e) {
        spdlog::error("get gui_enabled error: {}", e.what());
    }

    // Initialize QApplication if GUI is enabled, otherwise QCoreApplication
    if (GUIenabled) {
        mApp = std::make_unique<QApplication>(newArgc, argsPtrs.data());
    } else {
        mApp = std::make_unique<QCoreApplication>(newArgc, argsPtrs.data());
    }

//    spdlog::info("create qgis QgsApplication");
    mQgsApp = std::make_unique<QgsApplication>(newArgc, newArgv.release(), GUIenabled);
    QString qgis_prefix_path = "/usr";
    try {
        qgis_prefix_path = QString::fromStdString((*config)["qgis"]["prefix_path"].as<std::string>());
        spdlog::info("qgis_prefix_path: {}", qgis_prefix_path.toStdString());
    } catch (const std::exception& e) {
        spdlog::error("get qgis.prefix_path error: {}", e.what());
    }
    QgsApplication::setPrefixPath(qgis_prefix_path, true);

//    spdlog::info("init qgis app");
    try {
        QgsApplication::init();
        QgsApplication::initQgis();
        Qgs3D::initialize();
    } catch (const std::exception& e) {
        spdlog::error("init qgis error: {}", e.what());
    }
    /*mQSurfaceFormat = std::make_unique<QSurfaceFormat>();
    *mQSurfaceFormat = QSurfaceFormat::defaultFormat();
    mQSurfaceFormat->setVersion(4, 1);
    mQSurfaceFormat->setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(*mQSurfaceFormat);*/

    // 创建离屏渲染环境
    /*mQOffscreenSurface = std::make_unique<QOffscreenSurface>();
    mQOffscreenSurface->setFormat(*mQSurfaceFormat);
    mQOffscreenSurface->create();*/

    // 创建OpenGL上下文
    /*mOpenGLContext = std::make_unique<QOpenGLContext>();
    mOpenGLContext->setFormat(*mQSurfaceFormat);
    if (!mOpenGLContext->create()) {
        spdlog::error("Failed to create OpenGL context");
        exit(-1);
    }*/

    // 设置当前上下文
    /*if (!mOpenGLContext->makeCurrent(mQOffscreenSurface.get())) {
        spdlog::error("Failed to make OpenGL context current");
        exit(-1);
    }*/

//    spdlog::info("QCoreStarter Init end");
}

void QCoreStarter::Setup(StarterContext& context) {
//    spdlog::info("QCoreStarter Setup start");
//    spdlog::info("QCoreStarter Setup end");
}

void QCoreStarter::Start(StarterContext& context) {
//    spdlog::info("QCoreStarter Start start");
    auto config = context.Props();
    auto guiEnable = (*config)["qgis"]["gui_enabled"].as<bool>();
//    spdlog::info("start qgis app, guiEnable: {}", guiEnable);
    if (guiEnable) {
//        spdlog::info("enable gui, start gui application");
        QApplication::exec();
    } else {
//        spdlog::info("not enable gui, start core application");
        QCoreApplication::exec();
    }

//    spdlog::info("QCoreStarter Start end");
}

void QCoreStarter::Stop(StarterContext& context) {
    if (mStopped) {
        spdlog::info("QCoreStarter already stopped, skipping...");
        return;
    }
    // 停止事件循环
    if (!mStopped) {
        mStopped = true;
        if (QgsApplication::instance()) {
            QgsApplication::exitQgis();
        }
        //QCoreApplication::quit();
        QCoreApplication::exit(0);
    }


    // 释放 OpenGL 上下文
    /*if (mOpenGLContext) {
        if (mOpenGLContext->makeCurrent(mQOffscreenSurface.get())) {
            mOpenGLContext->doneCurrent();
        }
        mOpenGLContext->deleteLater();
        mOpenGLContext.reset();
    }*/

    // 释放离屏表面
    /*if (mQOffscreenSurface) {
        mQOffscreenSurface->destroy();
        mQOffscreenSurface.reset();
    }*/

//    spdlog::info("QCoreStarter Stop end");
}

int QCoreStarter::PriorityGroup() {
    return AppGroup;
}

bool QCoreStarter::StartBlocking() {
    return mBlock;  // 通常Web服务器启动会阻塞当前线程，可根据实际调整
}

std::array<int, 4> QCoreStarter::Priority() {
    return {2, 2, DEFAULT_PRIORITY, DEFAULT_PRIORITY};
}

std::string QCoreStarter::GetName() {
    return "QCoreStarter";
}

void QCoreStarter::SetBlocking(bool isBlock) {
    mBlock = isBlock;
}