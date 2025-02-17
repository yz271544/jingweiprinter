//
// Created by etl on 2/7/25.
//

#ifndef JINGWEIPRINTER_PROCESSOR_H
#define JINGWEIPRINTER_PROCESSOR_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <future>
#include <functional>
#include <yaml-cpp/yaml.h>
#include <QFile>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QTimer>
#include <qgsabstractgeometry.h>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <qgs3d.h>
#include <qgs3dmapcanvas.h>
#include <qgs3d.h>
#include <qgs3dmapcanvas.h>
#include <qgswkbtypes.h>
#include <qgslayoutmanager.h>
#include <qgsmapviewsmanager.h>
#include <qgsoffscreen3dengine.h>
#include <qgs3dutils.h>

#include "core/error/exceptions.h"
#include "core/enums/PaperSpecification.h"
#include "core/fetch/PlottingFetch.h"
#include "utils/Formula.h"
#include "utils/NodeToMap.h"
#include "utils/CompressUtil.h"
#include "core/qgis/layout/JwLayout.h"
#include "core/qgis/layout/JwLayout3D.h"
#include "App.h"
#include "utils/ShowDataUtil.h"
#include "layer/JwCircle.h"
#include "layer/JwPoint.h"
#include "layer/JwLine.h"
#include "layer/JwPolygon.h"
#include "utils/TypeConvert.h"

/*#include <QMetaType>
Q_DECLARE_METATYPE(QgsPoint)*/


class Processor {
private:
    bool m_enable_3d = true;
    bool m_verbose = false;
    bool m_export_png_enable = true;
    bool m_export_pdf_enable = false;
    bool m_export_svg_enable = false;
    YAML::Node *m_config;
    std::unique_ptr<PlottingFetch> m_plotting_fetch;
    std::unique_ptr<App> m_app;
    std::unique_ptr<QVariantMap> m_setting_image_spec;
    QString m_export_prefix;
    QString m_mapping_export_nginx_port;
    QString m_mapping_export_nginx_url_prefix;
public:
    // 构造函数
    Processor(const QList<QString>& argvList, YAML::Node* config);

    // 析构函数
    ~Processor();

    // 异步获取绘图数据的函数
    std::future<DTOWRAPPERNS::DTOWrapper<PlottingRespDto>>
    fetchPlotting(const oatpp::String &token, const oatpp::String &scene_type,
                  DTOWRAPPERNS::DTOWrapper<TopicMapData> &topic_map_data);


    // 检查和处理闭合几何图形的函数
    static void checkDealWithClosedGeometry(const DTOWRAPPERNS::DTOWrapper<GeoPolygonJsonDto> &geojson);

    // 异步处理绘图数据的函数
    std::future<DTOWRAPPERNS::DTOWrapper<ResponseDto>> processByPlottingWeb(
            const oatpp::String &token, const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingDto);

    // 绘制图层
    void plottingLayers(const DTOWRAPPERNS::DTOWrapper<PlottingRespDto> &plotting_data);

    // 添加2d布局
    void add_layout(QgsMapCanvas *canvas,
                    const QString &layout_name,
                    const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingWeb,
                    const QMap<QString, QVariant> &image_spec,
                    const PaperSpecification& available_paper,
                    bool write_qpt,
                    const QVector<QString> &removeLayerNames,
                    const QVector<QString> &removeLayerPrefixs);

    // 添加3d布局
    JwLayout3D* add_3d_layout(QgsMapCanvas* canvas,
                    const QString &layout_name,
                    const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingWeb,
                    const QMap<QString, QVariant> &image_spec,
                    const PaperSpecification& available_paper,
                    bool write_qpt,
                    const QVector<QString> &removeLayerNames,
                    const QVector<QString> &removeLayerPrefixes);

    // 导出PNG
    QString exportPNG(const QString &sceneName, const QString &layoutName, const QString &imageSubDir,
                 const QString &paperName);

    // 导出PDF
    QString exportPDF(const QString &sceneName, const QString &layoutName, const QString &imageSubDir,
                        const QString &paperName);

    // 导出SVG
    QString exportSVG(const QString &sceneName, const QString &layoutName, const QString &imageSubDir,
                        const QString &paperName);

    // 压缩项目的静态方法
    QString zipProject(const QString &scene_name);

    // 获取图像子目录
    QString getImageSubDir(const QString &layout_name);

    // 按颜色分组圆的函数
    /**
     * Grouped circle by color grouped
     * @param grouped_color { '#ff4040-#00cd52-#2f99f3': 2, '#1c6ad6-#00cd52-#cbc829': 1 }
     * @param polygon_geometry_coordinates_list 多个等级域圆心坐标list  exp: [[111.477486, 40.724372], [111.478305, 40.723215], [111.479145, 40.729253]]
     * @param polygon_geometry_properties_radius  exp: [41, 34, 91]
     * @param style_percents exp: [[40, 30, 30], [40, 30, 30], [40, 30, 30]]
     * @param areas_color_list [['#ff4040', '#00cd52', '#2f99f3'], ['#ff4040', '#00cd52', '#2f99f3'], ['#1c6ad6', '#00cd52', '#cbc829']]
     * @param areas_opacity_list [[0.4, 0.4, 0.4], [0.4, 0.4, 0.4], [0.6, 0.4, 0.5]]
     * @return {
            '#ff4040-#00cd52-#2f99f3': {
                "polygon_geometry_coordinates_list": [[111.477486, 40.724372], [111.478305, 40.723215]],
                "polygon_geometry_properties_radius": [41, 34],
                "style_percents": [[40, 30, 30], [40, 30, 30]],
                "areas_color_list": ['#ff4040', '#00cd52', '#2f99f3']
                "areas_opacity_list": [0.4, 0.4, 0.4]
            },
            '#1c6ad6-#00cd52-#cbc829': {
                "polygon_geometry_coordinates_list":  [[111.479145, 40.729253]],
                "polygon_geometry_properties_radius": [91],
                "style_percents": [[40, 30, 30]],
                "areas_color_list": ['#1c6ad6', '#00cd52', '#cbc829']
                "areas_opacity_list": [0.6, 0.4, 0.5]
        }
     */
    static QVariantMap _grouped_circle_by_color_grouped(
            QMap<QString, int>& grouped_color,
            QList<QList<double>>& polygon_geometry_coordinates_list,
            QList<int>& polygon_geometry_properties_radius,
            QList<QList<double>>& style_percents,
            QList<QList<QString>>& areas_color_list,
            QList<QList<double>>& areas_opacity_list);

    // 按相同颜色分组的函数
    /**
     * Group colors by the same color
     * @param name_list ['line1', 'line2', 'line3']
     * @param geometry_coordinates_list ['#ff4040', '#00cd52', '#2f99f3']
     * @param style_list [style1, style2, style3]
     * @return {
            '#ff4040': {
                "name_list": ['line1', 'line2'],
                "line_geometry_coordinates_list": [[111.477486, 40.724372], [111.478305, 40.723215]],
                "style_list": [style1, style2]
            },
            '#00cd52': {
                "name_list": ['line3', 'line4'],
                "line_geometry_coordinates_list": [[112.477486, 42.724372], [112.478305, 42.723215]],
                "style_list": [style3, style4]
            }
        }
     */
    static QVariantMap _grouped_color_lines(
            QList<QString> &name_list,
            QList<QList<QList<double>>> &geometry_coordinates_list,
            QList<QJsonObject> &style_list);


    static QVariantMap _grouped_color_polygons(
            QList<QString> &name_list,
            QList<QList<QList<QList<double>>>> &geometry_coordinates_list,
            QList<QJsonObject> &style_list);

    void export2DLayout(QString& sceneName,
                        const QString& layoutType,
                        const DTOWRAPPERNS::DTOWrapper<PlottingDto> &plottingWeb,
                        DTOWRAPPERNS::DTOWrapper<ResponseDto> responseDto);
};


#endif //JINGWEIPRINTER_PROCESSOR_H
