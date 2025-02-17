//
// Created by etl on 2/10/25.
//

#ifndef JINGWEIPRINTER_QCORESTARTER_H
#define JINGWEIPRINTER_QCORESTARTER_H

#include <QCoreApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <qgs3d.h>
#include "starter.h"
#include <spdlog/spdlog.h>
#include "yaml-cpp/yaml.h"


#include "confstarter.h"
#include "starter.h"
#include "starterregister.h"



#if defined(_WIN32) && defined(_MSC_VER)
#include "oatpp/core/base/Environment.hpp"
#endif

class QCoreStarter : public BaseStarter {
private:
    bool mBlock;
    bool mStopped = false;
//    std::unique_ptr<QOffscreenSurface> mQOffscreenSurface;
//    std::unique_ptr<QOpenGLContext> mOpenGLContext;
    std::unique_ptr<QCoreApplication> mApp;
    std::unique_ptr<QgsApplication> mQgsApp;
//    std::unique_ptr<QSurfaceFormat> mQSurfaceFormat;
public:
    QCoreStarter();

    ~QCoreStarter();

    BaseStarter* GetInstance();

    void Init(StarterContext& context);

    void Setup(StarterContext& context);

    void Start(StarterContext& context);

    void Stop(StarterContext& context);

    int PriorityGroup();

    bool StartBlocking();

    std::array<int, 4> Priority();

    std::string GetName();

    void SetBlocking(bool isBlock);
};



#endif //JINGWEIPRINTER_QCORESTARTER_H
