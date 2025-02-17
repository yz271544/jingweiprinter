//
// Created by etl on 24-12-18.
//

#ifndef LOGGERSTARTER_H
#define LOGGERSTARTER_H

#include "spdlog/spdlog.h"
#include "starter.h"
#include "yaml-cpp/yaml.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QDateTime>
#include <QThread>

class LoggerStarter : public BaseStarter {
private:

public:
    // 构造函数，可以在这里进行一些初始的配置或者资源准备工作，如果不需要可以保持默认实现
    LoggerStarter();

    // 析构函数，用于释放相关资源，目前示例中暂未涉及复杂资源释放，可按需完善
    ~LoggerStarter();

    BaseStarter* GetInstance();

    // 获取单例实例的静态方法
    static LoggerStarter* getInstance() {
        static LoggerStarter instance;
        return &instance;
    }

    // 初始化方法，从指定路径加载配置文件，若加载失败会输出错误信息
    void Init(StarterContext& context);

    // 配置相关的设置操作，留给具体业务逻辑根据需求去实现更详细的配置调整等功能
    void Setup(StarterContext& context);

    // 启动相关逻辑，根据从配置文件中解析出的配置内容做一些初始化启动相关的操作，需按实际业务需求实现
    void Start(StarterContext& context);

    // 停止相关操作，清理与配置相关的资源或者状态等，同样需按具体业务逻辑完善
    void Stop(StarterContext& context);

    // 返回该启动器所属的优先级分组，这里返回基础资源组（BasicResourcesGroup）
    int PriorityGroup();

    // 指示该启动器启动时是否阻塞，这里返回false，表示非阻塞启动
    bool StartBlocking();

    // 返回该启动器的优先级数值，使用默认优先级（DEFAULT_PRIORITY）
    std::array<int, 4> Priority();

    // 获取启动器的名称，方便在日志、调试或者管理启动器列表等场景使用
    std::string GetName();

    static void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
private:
    bool mStopped = false;

    void setQLoggerLevel(std::string& level);

    void disableAllQtLogs();
};



#endif //LOGGERSTARTER_H
