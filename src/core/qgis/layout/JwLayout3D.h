﻿//
// Created by etl on 2/3/25.
//

#ifndef CBOOT_JWLAYOUT3D_H
#define CBOOT_JWLAYOUT3D_H


#include <qgis.h>
#include <QDebug>
#include <QVector>
#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QScreen>
#include <QSplashScreen>
#include <QStringLiteral>
#include <Qt3DCore>
#include <QtConcurrent/QtConcurrent>

#include <qentity.h>
#include <qgs3dmapcanvas.h>
#include <qgsproject.h>
#include <qgslayout.h>
#include <qgsprintlayout.h>
#include <qgslayoutitemmap.h>
#include <qgslayoutpagecollection.h>
#include <qgslayoutmanager.h>
#include <qgslayoutitemlegend.h>
#include <qgslayoutitemlabel.h>
#include <qgslayoutitempicture.h>
#include <qgslayoutitemscalebar.h>
#include <qgslayoutitempolyline.h>
#include <qgslayoutpoint.h>
#include <qgslayoutsize.h>
#include <qgslayoutmeasurement.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsfillsymbol.h>
#include <qgslinesymbol.h>
#include <qgsarrowsymbollayer.h>
#include <qgsreadwritecontext.h>
#include <qgsmapcanvas.h>
#include <qgs3dmapsettings.h>
#include <qgslayoutitem3dmap.h>
#include <qgscamerapose.h>
#include <qgsvector3d.h>
#include <qgslayoutitemshape.h>
#include <qgssymbollayer.h>
#include <qgssymbollayerwidget.h>
#include <qgsmarkersymbollayer.h>
#include <qgsprojectviewsettings.h>
#include <qgsflatterraingenerator.h>
#include <qgsprojectelevationproperties.h>
#include <qgsterrainprovider.h>
#include <qgspointlightsettings.h>
#include <qgs3dmapscene.h>
#include <qgsmapviewsmanager.h>
#include <qgsconfig.h>
#include <qgs3dutils.h>
#include <qgssettings.h>
#include <qgsdirectionallightsettings.h>
#include <qgslayoutexporter.h>
#include <qgsfillsymbollayer.h>

//#include <qgis/app/3d/qgs3dmapcanvaswidget.h>
#include "JwLegend.h"
#include "core/enums/PaperSpecification.h"
#include "utils/QgsUtil.h"
#include "utils/ColorTransformUtil.h"
#include "utils/ImageUtil.h"
#include "core/qgis/d3/CameraUtil.h"
#include "core/handler/dto/plotting.h"
#include "config.h"

class JwLayout3D {

public:
    JwLayout3D(QgsProject *project,
               QgsMapCanvas *canvas2d,
               Qgs3DMapCanvas *canvas3d,
               QString &sceneName,
               const QVariantMap &imageSpec,
               QString &projectDir,
               QString &layoutName,
               QString &qgisPrefixPath);

    ~JwLayout3D() = default;

    void filterMapLayers(const QVector<QString> &removeLayerNames = QVector<QString>(),
                         const QVector<QString> &removeLayerPrefixes = QVector<QString>(),
                         Qgs3DMapSettings *mapSettings3d = nullptr);

    static void setPageOrientation(
            QgsPrintLayout *layout,
            const PaperSpecification &availablePaper,
            int pageNum,
            QgsLayoutItemPage::Orientation orientation = QgsLayoutItemPage::Landscape);

    void setTitle(QgsPrintLayout *layout, const QVariantMap &titleOfLayinfo);

    void setLegend(
            QgsPrintLayout *layout,
            const QVariantMap &imageSpec,
            int legendWidth = 40,
            int legendHeight = 80,
            const QString &borderColor = "#000000",
            const QSet<QString> &filteredLegendItems = QSet<QString>());

    void setRemarks(QgsPrintLayout *layout,
                    const QVariantMap &remarkOfLayinfo,
                    const bool writeQpt);

    void addRightSideLabel(
            QgsPrintLayout *layout,
            const QVariantMap &subTitle,
            int rightSideLabelWidth,
            int rightSideLabelHeight);

    void addSignatureLabel(
            QgsPrintLayout *layout,
            const QString &signatureText);

    static void addArrowToLayout(
            QgsLayout *layout,
            const QVector<QgsPointXY> &points,
            const QColor &color, double width);

    void addArrowBasedOnFrontendParams(
            QgsLayout *layout,
            const QList<QVariant> &position, double rotate);

    void init3DLayout(const QString &layoutName);

    void init3DMapSettings(
            const QVector<QString> &removeLayerNames,
            const QVector<QString> &removeLayerPrefixes
    );

    /**
     * 设置 3D 地图相机参数
     * @param cameraLongitude 摄像机经度
     * @param cameraLatitude 摄像机纬度
     * @param cameraHeight 摄像机高度
     * @param cameraDirX 摄像机方向向量 X 分量
     * @param cameraDirY 摄像机方向向量 Y 分量
     * @param cameraDirZ 摄像机方向向量 Z 分量
     * @param cameraUpX 摄像机上方向向量 X 分量
     * @param cameraUpY 摄像机上方向向量 Y 分量
     * @param cameraUpZ 摄像机上方向向量 Z 分量
     * @param cameraRightX 摄像机右方向向量 X 分量
     * @param cameraRightY 摄像机右方向向量 Y 分量
     * @param cameraRightZ 摄像机右方向向量 Z 分量
     * @param fov 垂直视场角
     * @param aspectRatio 长宽比
     * @param nearPlane 近裁剪面
     * @param farPlane 远裁剪面
     */
    void set3DCanvas(DTOWRAPPERNS::DTOWrapper<Camera3dPosition>& camera, double default_distance);

    void setTest3DCanvas();

    void set3DMap(
            QgsPrintLayout *layout,
            const PaperSpecification &availablePaper,
            int mapFrameWidth = 1,
            const QString &mapFrameColor = "#000000",
            bool isDoubleFrame = false,
            double mapRotation = 0.0
    );


    void addNorthArrow(
            QgsPrintLayout *layout,
            const QVariantMap &northArrowPath);

    void addPrintLayout(const QString &layoutType, const QString &layoutName,
                        const QVariantMap &plottingWeb, const PaperSpecification &availablePaper,
                        bool writeQpt = false);

    void loadQptTemplate(const QString &qptFilePath, const QString &layoutTemplateName);

    void updateLayoutExtent(const QString &layoutName);

    QPair<double, double> getLegendDimensions(const QString &layoutName);

    void exportLayoutToPng(
            const QString &layoutName,
            QString &outputPath);

    void exportLayoutToPdf(
            const QString &layoutName,
            QString &outputPath);

    void exportLayoutToSvg(
            const QString &layoutName,
            QString &outputPath);

    QgsPrintLayout *getLayout(const QString &layoutName);

    void saveQptTemplate(QgsLayout *layout);

    QgsLayoutItem3DMap *getMapItem3d();

    Qgs3DMapSettings *getMapSettings3d();

    QgsPrintLayout *getLayout3D();

    void destroy3DCanvas();

    QgsLayoutItemShape *addRect(
            QString &fillColor,
            const QString &borderColor,
            double borderWidth,
            qreal remarksX,
            qreal remarksY,
            qreal remarksWidth,
            qreal remarksHeight
    );

    static QgsPoint *transformPoint(const QgsPoint &point, const QgsCoordinateTransform &transformer);

    void setTestFrom2dExtent();

private:
    QgsProject *mProject;
    QString &mLayoutName;
    QString &mProjectDir;
    QgsMapCanvas *mCanvas2d;
    Qgs3DMapCanvas *mCanvas3d;
    QString &mSceneName;
    std::unique_ptr<JwLegend> mJwLegend;
    QgsPrintLayout *mLayout;
    QVariantMap mImageSpec;
    QgsLayoutItem3DMap *mMapItem3d;
    Qgs3DMapSettings *mMapSettings3d;
    QString &mQgisPrefixPath;
    double mMapWidth;
    double mMapHeight;
};


#endif //CBOOT_JWLAYOUT3D_H
