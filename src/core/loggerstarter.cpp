//
// Created by etl on 24-12-18.
//

#include "loggerstarter.h"

// ConfStarter类的构造函数，目前可保持默认实现，若后续有初始化相关成员变量等需求可在此添加代码
LoggerStarter::LoggerStarter() = default;

// ConfStarter类的析构函数，目前暂未涉及复杂资源释放，可按需完善，比如关闭相关文件流等（如果有使用的话）
LoggerStarter::~LoggerStarter() = default;

// 实现Init方法，从指定路径加载配置文件，若加载失败会输出错误信息到标准错误输出流
void LoggerStarter::Init(StarterContext &context) {
//    spdlog::info("ConfStarter Init start");
    bool qtLogEnable = false;
    try {
        auto config = context.Props();
        std::string loggerLevel = "info";
        try {
            loggerLevel = (*config)["logging"]["level"].as<std::string>();
            spdlog::warn("loggerLevel: {}", loggerLevel);

            qtLogEnable = (*config)["logging"]["qt_log_enable"].as<bool>();
            if (qtLogEnable) {
                setQLoggerLevel(loggerLevel);
                // 安装自定义消息处理程序
                qInstallMessageHandler(customMessageHandler);
            }
        } catch (const std::exception &e) {
            spdlog::warn("get logging.level error: {}", e.what());
        }
        auto level = spdlog::level::from_str(loggerLevel);
        spdlog::set_level(level);
        //spdlog::set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^---%L---%$] [thread %t] %v");
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S %z] [%^%l%$] [thread %t] %v");
    } catch (const YAML::BadFile &e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
    }
//    spdlog::info("ConfStarter Init end");
}

// 实现Setup方法，目前此方法只是一个占位，可根据具体业务需求实现更详细的配置调整等功能
// 例如，根据配置内容对某些模块进行初始化设置，或者验证配置的合法性等
void LoggerStarter::Setup(StarterContext &context) {
//    spdlog::info("ConfStarter Setup start");
//    spdlog::info("ConfStarter Setup end");
}

// 实现Start方法，根据从配置文件中解析出的配置内容做一些初始化启动相关的操作，需按实际业务需求实现
// 比如，根据配置启动相应的服务、初始化数据库连接等（取决于具体应用场景）
void LoggerStarter::Start(StarterContext &context) {
//    spdlog::info("ConfStarter Start start");
//    spdlog::info("ConfStarter Start end");
}

// 实现Stop方法，清理与配置相关的资源或者状态等，同样需按具体业务逻辑完善
// 例如，关闭打开的配置文件（如果有保持打开状态的情况），释放相关内存资源等
void LoggerStarter::Stop(StarterContext &context) {
    if (mStopped) {
//        spdlog::info("QCoreStarter already stopped, skipping...");
        return;
    }
    mStopped = true;
//    spdlog::info("ConfStarter Stop start");
//    spdlog::info("ConfStarter Stop end");
}

// 返回该启动器所属的优先级分组，这里按照定义返回基础资源组（BasicResourcesGroup）
int LoggerStarter::PriorityGroup() {
    return BasicResourcesGroup;
}

// 指示该启动器启动时是否阻塞，这里返回false，表示非阻塞启动
bool LoggerStarter::StartBlocking() {
    return false;
}

// 返回该启动器的优先级数值，使用默认优先级（DEFAULT_PRIORITY）
std::array<int, 4> LoggerStarter::Priority() {
    return {1, 1, 1, 2};
}

// 获取启动器的名称，方便在日志、调试或者管理启动器列表等场景使用
std::string LoggerStarter::GetName() {
    return "LoggerStarter";
}

// 实现Starter基类中获取启动器实例的抽象方法
BaseStarter *LoggerStarter::GetInstance() {
    return getInstance();
}

void LoggerStarter::setQLoggerLevel(std::string &level) {
    bool debug = false;
    bool info = false;
    bool warn = false;
    bool error = false;
    bool critical = false;
    bool fatal = false;

    // 根据输入的日志级别设置对应的标志
    if (level == "debug") {
        debug = true;
        info = true;
        warn = true;
        error = true;
        critical = true;
        fatal = true;
    } else if (level == "info") {
        info = true;
        warn = true;
        error = true;
        critical = true;
        fatal = true;
    } else if (level == "warn") {
        warn = true;
        error = true;
        critical = true;
        fatal = true;
    } else if (level == "error") {
        error = true;
        critical = true;
        fatal = true;
    } else if (level == "critical") {
        critical = true;
        fatal = true;
    } else if (level == "fatal") {
        fatal = true;
    } else {
        // 默认情况下，禁用所有日志
        debug = false;
        info = false;
        warn = false;
        error = false;
        critical = false;
        fatal = false;
    }

    // 构建 QLoggingCategory 的过滤规则
    QString qLoggerRule = QString(
            "qt.qpa.events.reader.warning=false\n"
            "*.debug=%1\n"
            "*.info=%2\n"
            "*.warning=%3\n"
            "*.error=%4\n"
            "*.critical=%5\n"
            "*.fatal=%6\n")
            .arg(debug ? "true" : "false")
            .arg(info ? "true" : "false")
            .arg(warn ? "true" : "false")
            .arg(error ? "true" : "false")
            .arg(critical ? "true" : "false")
            .arg(fatal ? "true" : "false");
    QLoggingCategory::setFilterRules(qLoggerRule);
}

// 全局关闭所有日志
static void disableAllQtLogs() {
    // 设置所有日志级别为 false
    QString qLoggerRule = QString("*.debug=false\n"
                                  "*.info=false\n"
                                  "*.warning=false\n"
                                  "*.error=false\n"
                                  "*.critical=false\n"
                                  "*.fatal=false\n");
    // 设置日志过滤规则
    QLoggingCategory::setFilterRules(qLoggerRule);
}

// 自定义消息处理函数
void LoggerStarter::customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString txt;
    // 根据日志级别添加相应的标识
    switch (type) {
        case QtDebugMsg:
            txt = QString("Debug: ");
            break;
        case QtInfoMsg:
            txt = QString("Info: ");
            break;
        case QtWarningMsg:
            txt = QString("Warning: ");
            break;
        case QtCriticalMsg:
            txt = QString("Critical: ");
            break;
        case QtFatalMsg:
            txt = QString("Fatal: ");
            abort();
    }

    // 添加时间戳
    txt += QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");

    // 添加线程ID
    txt += QString("Thread %1: ").arg((quintptr)QThread::currentThreadId());

    // 添加源文件和行号信息
    if (context.file != nullptr) {
        txt += QString("%1:%2 - ").arg(context.file).arg(context.line);
    }

    // 添加实际的日志消息
    txt += msg;

    // 输出日志到标准输出
    QByteArray localMsg = txt.toLocal8Bit();
    fprintf(stderr, "%s\n", localMsg.constData());
    fflush(stderr);
}