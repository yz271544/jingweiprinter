//
// Created by etl on 2/3/25.
//

#include "JwLayout3D.h"

#include <utility>

// 构造函数
JwLayout3D::JwLayout3D(QgsProject *project, QgsMapCanvas *canvas2d,
                       Qgs3DMapCanvas *canvas3d, QString &sceneName,
                       const QVariantMap &imageSpec, QString &projectDir,
                       QString &layoutName, QString &qgisPrefixPath)
    : mProject(project), mCanvas2d(canvas2d), mCanvas3d(canvas3d),
      mSceneName(sceneName), mImageSpec(imageSpec), mProjectDir(projectDir),
      mLayoutName(layoutName), mQgisPrefixPath(qgisPrefixPath), mMapWidth(0),
      mMapHeight(0) {
  QString legendTitle = imageSpec["legend_title"].toString();
  this->mJwLegend = std::make_unique<JwLegend>(legendTitle, project);
}

// 过滤地图图层
void JwLayout3D::filterMapLayers(const QVector<QString> &removeLayerNames,
                                 const QVector<QString> &removeLayerPrefixes,
                                 Qgs3DMapSettings *mapSettings3d) {
  for (const auto &item : removeLayerNames) {
    spdlog::debug("remove layer name: {}", item.toStdString());
  }

  for (const auto &item : removeLayerPrefixes) {
    spdlog::debug("remove layer prefix: {}", item.toStdString());
  }

  QMap<QString, QgsMapLayer *> layers = mProject->mapLayers();
  QList<QgsMapLayer *> filteredLayers;
  for (QgsMapLayer *layer : layers) {
    bool shouldAdd = true;
    if (!removeLayerNames.isEmpty() &&
        removeLayerNames.contains(layer->name())) {
      shouldAdd = false;
    }
    if (shouldAdd && !removeLayerPrefixes.isEmpty()) {
      for (const QString &prefix : removeLayerPrefixes) {
        if (layer->name().startsWith(prefix)) {
          shouldAdd = false;
          break;
        }
      }
    }
    if (shouldAdd) {
      filteredLayers.append(layer);
    }
  }
  if (mapSettings3d) {
    spdlog::debug("set layers to 3d map settings");
    std::reverse(filteredLayers.begin(), filteredLayers.end());
    for (QgsMapLayer *filtered_layer : filteredLayers) {
      spdlog::debug("JwLayout3D add layer to layout map: {}",
                    filtered_layer->name().toStdString());
    }
    mapSettings3d->setLayers(filteredLayers);
    spdlog::debug("set layers to 3d map settings done");
  }
}

// 设置页面方向
void JwLayout3D::setPageOrientation(
    QgsPrintLayout *layout, const PaperSpecification &availablePaper,
    int pageNum, QgsLayoutItemPage::Orientation orientation) {
  QgsLayoutPageCollection *pageCollection = layout->pageCollection();
  QgsLayoutItemPage *page = pageCollection->page(pageNum);
  page->setPageSize(availablePaper.getPaperName(), orientation);
}

void JwLayout3D::setTitle(QgsPrintLayout *layout,
                          const QVariantMap &titleOfLayinfo) {
  // 添加标题
  auto title = std::make_unique<QgsLayoutItemLabel>(layout);
  title->setText(titleOfLayinfo["text"].toString());

  // 设置标题字号
  int titleFontSize = mImageSpec["title_font_size"].toInt();
  if (titleOfLayinfo.contains("fontSize")) {
    titleFontSize = titleOfLayinfo["fontSize"].toInt();
  }

  // 字体
  QString fontFamily = mImageSpec["title_font_family"].toString();
  if (titleOfLayinfo.contains("fontFamily")) {
    fontFamily = titleOfLayinfo["fontFamily"].toString();
  }

  // 字体颜色
  QString fontColor = mImageSpec["title_font_color"].toString();
  if (titleOfLayinfo.contains("color")) {
    fontColor = titleOfLayinfo["color"].toString();
    fontColor = ColorTransformUtil::strRgbaToHex(fontColor).first;
  }

  // set the font
  QgsTextFormat text_format = *QtFontUtil::createFont(
      fontFamily, titleFontSize, fontColor,
      mImageSpec["title_is_bold"].toBool(),
      mImageSpec["title_is_italic"].toBool(), Qgis::TextOrientation::Horizontal,
      mImageSpec["title_letter_spacing"].toDouble());

  title->setVAlign(Qt::AlignBottom);
  title->setHAlign(Qt::AlignHCenter);
  title->setTextFormat(text_format);
  spdlog::debug("setTextFormat done");
  title->adjustSizeToText();

  spdlog::debug("title_font_size: {}, title_font_family: {}, title_font_color: "
                "{}, title_letter_spacing: {}",
                titleFontSize, fontFamily.toStdString(),
                fontColor.toStdString(),
                mImageSpec["title_letter_spacing"].toDouble());
  title->attemptSetSceneRect(
      QRectF(mImageSpec["main_left_margin"].toDouble(), 0.0, mMapWidth,
             mImageSpec["main_top_margin"].toDouble() - 10));
  layout->addLayoutItem(title.release());
}

// 添加图例
void JwLayout3D::setLegend(QgsPrintLayout *layout, const QVariantMap &imageSpec,
                           int legendWidth, int legendHeight,
                           const QString &borderColor,
                           const QSet<QString> &filteredLegendItems) {
  auto legend = std::make_unique<QgsLayoutItemLegend>(layout);
  spdlog::debug("ready to custom legend for 3d");
  QPair<int, int> legendWidthHeight = mJwLegend->customize(
      legend.get(), imageSpec, legendWidth, legendHeight, filteredLegendItems);
  spdlog::debug("custom legend done");
  legendWidth = legendWidthHeight.first;
  legendHeight = legendWidthHeight.second;
  double legendX =
      imageSpec["main_left_margin"].toDouble() + mMapWidth - legendWidth - 0.5;
  spdlog::debug(
      "legend_x: {}, main_left_margin: {}, map_width:{}, legend_width: {}",
      legendX, imageSpec["main_left_margin"].toDouble(), mMapWidth,
      legendWidth);
  double legendY =
      imageSpec["main_top_margin"].toDouble() + mMapHeight - legendHeight - 0.5;
  spdlog::debug(
      "legend_y: {}, main_top_margin: {}, map_height: {}, legend_height: {}",
      legendY, imageSpec["main_top_margin"].toDouble(), mMapHeight,
      legendHeight);
  // auto sizeOfLegend = legend->sizeWithUnits();
  // auto rectOfLegend = legend->rectWithFrame();
  // spdlog::info("sizeOfLegend.height: {}, sizeOfLegend.width: {},
  // sizeOfLegend.units: {}", sizeOfLegend.height(), sizeOfLegend.width(),
  // sizeOfLegend.units()); spdlog::info("rectOfLegend.height: {},
  // rectOfLegend.width: {}", rectOfLegend.height(), rectOfLegend.width());
  // spdlog::info("legendHeight: {}, mMapHeight: {}", legendHeight, mMapHeight);
  // if (legendHeight * 2 > mMapHeight)
  // {
  //     legend->setColumnCount(2);
  // }
  legend->setColumnCount(8);
  legend->setResizeToContents(true);
  legend->setReferencePoint(QgsLayoutItem::ReferencePoint::LowerRight);
  spdlog::debug("set_legend legend_x: {}, legend_y: {}, legend_width: {}, "
                "legend_height: {}",
                legendX, legendY, legendWidth, legendHeight);
  legend->attemptSetSceneRect(
      QRectF(legendX, legendY, legendWidth, legendHeight));
  // legend->setFixedSize(QgsLayoutSize());
  legend->setFrameEnabled(true);
  legend->setFrameStrokeWidth(
      QgsLayoutMeasurement(0.5, Qgis::LayoutUnit::Millimeters));
  legend->setFrameStrokeColor(QColor(borderColor));
  layout->addLayoutItem(legend.release());
}

void JwLayout3D::setRemarks(QgsPrintLayout *layout,
                            const QVariantMap &remarkOfLayInfo,
                            const bool writeQpt) {
  // 获取备注文本
  QString remarkText = remarkOfLayInfo["text"].toString();

  // 获取备注位置信息 [左, 上, 宽, 高]
  QVariantList position = remarkOfLayInfo["position"].toList();
  double positionLeft = position[0].toDouble();
  double positionTop = position[1].toDouble();
  double positionWidth = position[2].toDouble();
  double positionHeight = position[3].toDouble();

  // 计算备注框的宽度和高度
  int remarksWidth = static_cast<int>(mMapWidth * positionWidth / 100.0);
  int remarksHeight = static_cast<int>(mMapHeight * positionHeight / 100.0);

  // 获取字体大小
  double remarkFontSize = remarkOfLayInfo.contains("fontSize")
                              ? remarkOfLayInfo["fontSize"].toDouble()
                              : mImageSpec["remark_font_size"].toDouble();

  // 计算文本宽度和高度
  int remarksTextWidth =
      FontUtil::getTextFontWidth(remarkText, remarkFontSize,
                                 mImageSpec["remark_letter_spacing"].toFloat());
  int remarksTextHeight = FontUtil::getSingleTextSize(remarkFontSize);

  // 调整备注框的宽度和高度
  remarksWidth = qMax(remarksTextWidth, remarksWidth);
  remarksHeight = qMax(remarksTextHeight, remarksHeight);

  // 计算备注框的坐标
  double remarksX = mImageSpec["main_left_margin"].toDouble() + 0.25;
  if (positionLeft > 0) {
    remarksX += mMapWidth * positionLeft / 100.0;
  }
  double remarksY = mImageSpec["main_top_margin"].toDouble() +
                    mMapHeight * positionTop / 100.0 - 0.25;

  // 获取背景颜色和边框颜色
  QString bgColor = remarkOfLayInfo.contains("fillColor") &&
                            !remarkOfLayInfo["fillColor"].toString().isEmpty()
                        ? ColorTransformUtil::strRgbaToHex(
                              remarkOfLayInfo["fillColor"].toString())
                              .first
                        : mImageSpec["remark_bg_color"].toString();
  QString bgFrameColor =
      remarkOfLayInfo.contains("borderColor") &&
              !remarkOfLayInfo["borderColor"].toString().isEmpty()
          ? remarkOfLayInfo["borderColor"].toString()
          : mImageSpec["remark_bg_frame_color"].toString();

  // 创建备注框背景
  auto remarksBg = std::make_unique<QgsLayoutItemShape>(layout);
  remarksBg->setShapeType(QgsLayoutItemShape::Rectangle);

  // 设置背景颜色和边框颜色
  auto symbol = std::make_unique<QgsFillSymbol>();
  symbol->setColor(QColor(bgColor));
  if (auto *symbolLayer =
          dynamic_cast<QgsSimpleMarkerSymbolLayer *>(symbol->symbolLayer(0))) {
    symbolLayer->setStrokeColor(QColor(bgFrameColor));
  }
  remarksBg->setSymbol(symbol.release());

  // 设置备注框的位置和大小
  remarksBg->setReferencePoint(QgsLayoutItem::ReferencePoint::UpperLeft);
  remarksBg->attemptSetSceneRect(
      QRectF(remarksX, remarksY, remarksWidth, remarksHeight));
  layout->addLayoutItem(remarksBg.release());

  // 创建备注文本
  auto remarks = std::make_unique<QgsLayoutItemLabel>(layout);
  remarks->setText(remarkText);

  // 设置字体格式
  QgsTextFormat *remarksFont = QtFontUtil::createFont(
      mImageSpec["remark_font_family"].toString(), remarkFontSize,
      ColorTransformUtil::strRgbaToHex(
          remarkOfLayInfo.contains("color")
              ? remarkOfLayInfo["color"].toString()
              : mImageSpec["remark_font_color"].toString())
          .first,
      mImageSpec["remark_is_bold"].toBool(),
      mImageSpec["remark_is_italic"].toBool(),
      Qgis::TextOrientation::
          Horizontal, // static_cast<Qt::Orientation>(imageSpec["remarkOrientation"].toInt()),
      mImageSpec["remark_letter_spacing"].toDouble());
  remarks->setTextFormat(*remarksFont);

  // 设置文本对齐方式
  remarks->setVAlign(Qt::AlignTop);
  remarks->setHAlign(Qt::AlignLeft);

  // 调整文本大小
  remarks->adjustSizeToText();

  // 设置文本位置
  remarks->setReferencePoint(QgsLayoutItem::ReferencePoint::UpperLeft);
  remarks->attemptSetSceneRect(QRectF(remarksX + 1, remarksY + 1,
                                      remarks->boundingRect().width(),
                                      remarks->boundingRect().height()));

  // 添加备注文本到布局
  layout->addLayoutItem(remarks.release());

  // 刷新布局
  layout->refresh();

  // 保存为 .qpt 文件
  if (writeQpt) {
    QString qptFilePath = mProjectDir + "/legend.qpt";
    QgsReadWriteContext context;
    layout->saveAsTemplate(qptFilePath, context);
  }
}

void JwLayout3D::addRightSideLabel(QgsPrintLayout *layout,
                                   const QVariantMap &subTitle,
                                   int rightSideLabelWidth,
                                   int rightSideLabelHeight,
                                   bool mapDoubleFrame,
                                   float margin_offset_if_double_frame) {
  // 创建标签项
  auto label = std::make_unique<QgsLayoutItemLabel>(layout);

  // 设置标签文本
  QString labelText = subTitle["text"].toString();
  label->setText(labelText);

  // 获取字体大小和颜色
  double rightSeamSealFontSize =
      subTitle.contains("fontSize")
          ? subTitle["fontSize"].toDouble()
          : mImageSpec["right_seam_seal_font_size"].toDouble();
  QString rightSeamSealFontColor =
      subTitle.contains("color")
          ? ColorTransformUtil::strRgbaToHex(subTitle["color"].toString()).first
          : mImageSpec["right_seam_seal_font_color"].toString();

  // 计算文本宽度和高度
  double textFontHeight = FontUtil::getTextFontWidth(
      labelText, rightSeamSealFontSize,
      mImageSpec["signatureLetterSpacing"].toFloat());
  double textFontWidth = FontUtil::getSingleTextSize(rightSeamSealFontSize);

  // 打印调试信息
  spdlog::debug("add_right_side_label font {}, size: {}, color: {}, "
                "right_seam_seal_letter_spacing: {},"
                " text_font_width: {}, text_font_height: {}",
                mImageSpec["right_seam_seal_family"].toString().toStdString(),
                rightSeamSealFontSize, rightSeamSealFontColor.toStdString(),
                mImageSpec["right_seam_seal_letter_spacing"].toDouble(),
                textFontWidth, textFontHeight);

  // 创建字体格式
  QgsTextFormat textFormat;
  QFont font(mImageSpec["right_seam_seal_family"].toString(),
             static_cast<int>(rightSeamSealFontSize));
  font.setLetterSpacing(
      QFont::AbsoluteSpacing,
      mImageSpec["right_seam_seal_letter_spacing"].toDouble());
  textFormat.setFont(font);
  textFormat.setColor(QColor(rightSeamSealFontColor));
  textFormat.setOrientation(
      Qgis::TextOrientation::Vertical); // 设置文本方向为垂直

  // 设置标签的字体格式
  label->setTextFormat(textFormat);

  // 设置垂直居中对齐
  label->setVAlign(Qt::AlignVCenter);

  // 计算标签的位置
  double labelX =
      mImageSpec["main_left_margin"].toDouble() + mMapWidth +
      mImageSpec["right_seam_seal_from_right_border_left_margin"].toDouble();
  double labelY =
      mImageSpec["main_top_margin"].toDouble() +
      mImageSpec["right_seam_seal_from_right_border_left_margin"].toDouble();

  // 设置标签的位置和大小
  if (mapDoubleFrame) {
    label->attemptSetSceneRect(QRectF(labelX + margin_offset_if_double_frame,
                                      labelY, textFontWidth, textFontHeight));
  } else {
    label->attemptSetSceneRect(
        QRectF(labelX, labelY, textFontWidth, textFontHeight));
  }

  // 将标签添加到布局
  layout->addLayoutItem(label.release());
}

void JwLayout3D::addSignatureLabel(QgsPrintLayout *layout,
                                   const QString &signatureText,
                                   bool mapDoubleFrame,
                                   float margin_offset_if_double_frame) {
  // 创建标签项
  auto label = std::make_unique<QgsLayoutItemLabel>(layout);

  // 设置标签文本
  label->setText(signatureText);

  // 计算文本宽度和高度
  double textFontWidth = FontUtil::getTextFontWidth(
      signatureText, mImageSpec["signature_font_size"].toDouble(),
      mImageSpec["signature_letter_spacing"].toFloat());
  double textFontHeight =
      FontUtil::getSingleTextSize(mImageSpec["signature_font_size"].toDouble());

  // 打印调试信息
  spdlog::debug(
      "add_signature_label font: {}, size: {}, color: {}, letter_spacing: {}, "
      "text_font_width: {}, text_font_height: {}",
      mImageSpec["right_seam_seal_family"].toString().toStdString(),
      mImageSpec["right_seam_seal_font_size"].toDouble(),
      mImageSpec["right_seam_seal_font_color"].toString().toStdString(),
      mImageSpec["right_seam_seal_letter_spacing"].toDouble(), textFontWidth,
      textFontHeight);

  // 创建字体格式
  QgsTextFormat signatureTextFormat;
  QFont font(mImageSpec["signature_family"].toString(),
             mImageSpec["signature_font_size"].toInt());
  font.setLetterSpacing(QFont::AbsoluteSpacing, 3.0); // 设置字间距
  signatureTextFormat.setFont(font);
  signatureTextFormat.setColor(
      QColor(mImageSpec["signatureFontColor"].toString()));
  signatureTextFormat.setOrientation(
      Qgis::TextOrientation::Horizontal); // 设置文本方向为水平

  // 设置标签的字体格式
  label->setTextFormat(signatureTextFormat);

  // 设置垂直和水平居中对齐
  label->setVAlign(Qt::AlignVCenter);
  label->setHAlign(Qt::AlignHCenter);

  // 计算标签的位置
  double labelX =
      mImageSpec["main_left_margin"].toDouble() + mMapWidth - textFontWidth -
      mImageSpec["signature_from_right_border_right_margin"].toDouble();
  double labelY =
      mImageSpec["main_top_margin"].toDouble() + mMapHeight +
      mImageSpec["signature_from_bottom_border_top_margin"].toDouble();

  // 设置标签的位置和大小
  if (mapDoubleFrame) {
    label->attemptSetSceneRect(QRectF(labelX,
                                      labelY + margin_offset_if_double_frame,
                                      textFontWidth, textFontHeight));
  } else {
    label->attemptSetSceneRect(
        QRectF(labelX, labelY, textFontWidth, textFontHeight));
  }

  // 将标签添加到布局
  layout->addLayoutItem(label.release());
}

void JwLayout3D::addArrowToLayout(QgsLayout *layout,
                                  const QVector<QgsPointXY> &points,
                                  const QColor &color, double width) {
  if (points.isEmpty()) {
    return;
  }
  // 将 QgsPointXY 转换为 QPolygonF
  QPolygonF polygon;
  for (const QgsPointXY &point : points) {
    polygon.append(QPointF(point.x(), point.y()));
  }

  // 创建多线段项
  auto polylineItem = std::make_unique<QgsLayoutItemPolyline>(polygon, layout);

  // 创建线符号
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple(
      QVariantMap{{"color", color.name()}, {"width", QString::number(width)}});

  // 创建箭头符号层
  auto arrowSymbolLayer = std::make_unique<QgsArrowSymbolLayer>();
  arrowSymbolLayer->setIsCurved(false);
  arrowSymbolLayer->setArrowType(QgsArrowSymbolLayer::ArrowPlain);
  arrowSymbolLayer->setColor(color);
  arrowSymbolLayer->setWidth(width);
  arrowSymbolLayer->setWidthUnit(Qgis::RenderUnit::Points);
  arrowSymbolLayer->setStrokeColor(color);
  arrowSymbolLayer->setFillColor(color);

  // 将箭头符号层添加到线符号
  lineSymbol->changeSymbolLayer(0, arrowSymbolLayer.release());

  // 设置线符号到多线段项
  polylineItem->setSymbol(lineSymbol);

  // 将多线段项添加到布局
  layout->addLayoutItem(polylineItem.release());

  // 打印调试信息
  spdlog::debug("Arrow added to layout with color: {}, width: {}",
                color.name().toStdString(), width);
}

void JwLayout3D::addArrowBasedOnFrontendParams(QgsLayout *layout,
                                               const QList<QVariant> &position,
                                               double rotate) {
  // 检查位置参数是否有效
  if (position.size() < 4) {
    spdlog::warn("Invalid position parameters. Expected 4 values: [left, top, "
                 "width, height].");
    return;
  }

  // 将相对比例坐标转换为毫米坐标（左上角坐标）
  double left = mImageSpec["main_left_margin"].toDouble() +
                mMapWidth * (position[0].toDouble() / 100.0);
  double top = mImageSpec["main_top_margin"].toDouble() +
               mMapHeight * (position[1].toDouble() / 100.0);

  // 获取图片相对比例表示的宽和高转换为实际毫米尺寸
  double widthMm = mMapWidth * (position[2].toDouble() / 100.0);
  double heightMm = mMapHeight * (position[3].toDouble() / 100.0);

  // 计算箭头起始坐标（无旋转时起点坐标）
  double startX = left + widthMm;
  double startY = top + heightMm / 2.0;

  // 计算箭头结束坐标（无旋转时终点坐标）
  double endX = left;
  double endY = top + heightMm / 2.0;

  // 根据旋转角度计算旋转后的起止点坐标
  double angleRad = rotate * M_PI / 180.0; // 将角度转换为弧度
  double dxStart = (startX - left) * std::cos(angleRad) -
                   (startY - top) * std::sin(angleRad);
  double dyStart = (startX - left) * std::sin(angleRad) +
                   (startY - top) * std::cos(angleRad);
  double rotatedStartX = left + dxStart;
  double rotatedStartY = top + dyStart;

  double dxEnd =
      (endX - left) * std::cos(angleRad) - (endY - top) * std::sin(angleRad);
  double dyEnd =
      (endX - left) * std::sin(angleRad) + (endY - top) * std::cos(angleRad);
  double rotatedEndX = left + dxEnd;
  double rotatedEndY = top + dyEnd;

  // 构建用于绘制箭头的点坐标列表（毫米坐标形式）
  QVector<QgsPointXY> points = {QgsPointXY(rotatedStartX, rotatedStartY),
                                QgsPointXY(rotatedEndX, rotatedEndY)};

  // 调用添加箭头到布局的函数
  addArrowToLayout(layout, points, Qt::red, 1.0);

  // 打印调试信息
  spdlog::debug("Arrow added based on frontend params -> position: "
                "{}:{}:{}:{}, rotate: {}",
                position[0].toDouble(), position[1].toDouble(),
                position[2].toDouble(), position[3].toDouble(), rotate);
}

void JwLayout3D::init3DLayout(const QString &layoutName) {
  // mLayout = std::make_unique<QgsPrintLayout>(mProject);
  mLayout = new QgsPrintLayout(mProject);
  mLayout->setName(layoutName);
  mLayout->setUnits(Qgis::LayoutUnit::Millimeters);
  mLayout->initializeDefaults();
  QgsLayoutManager *layout_manager = mProject->layoutManager();
  layout_manager->addLayout(mLayout);
}

// get layout from project->layoutManager()
QgsPrintLayout *JwLayout3D::getLayout(const QString &layoutName) {
  auto layout_manager = mProject->layoutManager();
  auto layoutInterface = layout_manager->layoutByName(layoutName);
  if (!layoutInterface) {
    spdlog::warn("Layout not found: {}", layoutName.toStdString());
    return nullptr;
  }
  if (layoutInterface->layoutType() == QgsMasterLayoutInterface::PrintLayout) {
    auto printLayout = dynamic_cast<QgsPrintLayout *>(layoutInterface);
    return printLayout;
  }
  return nullptr;
}

QgsPrintLayout *JwLayout3D::getLayout3D() { return mLayout; }

QgsLayoutItem3DMap *JwLayout3D::getMapItem3d() {
  //    auto layout = getLayout(mLayoutName);
  auto layout = getLayout3D();
  auto itemMap = layout->referenceMap();
  spdlog::debug("try reinterpret_cast<QgsLayoutItem3DMap*>");
  auto item3dMap = reinterpret_cast<QgsLayoutItem3DMap *>(itemMap);
  spdlog::debug("try reinterpret_cast<QgsLayoutItem3DMap*> done");
  return item3dMap;
}

void JwLayout3D::saveQptTemplate(QgsLayout *layout) {
  spdlog::debug("保存为 .qpt 文件");
  QString qptFilePath =
      QString("%1/%2%3").arg(mProjectDir, mLayoutName, ".qpt");
  QgsReadWriteContext context;
  layout->saveAsTemplate(qptFilePath, context);
  spdlog::debug("Saved layout as QPT template: {}", qptFilePath.toStdString());
}

Qgs3DMapSettings *JwLayout3D::getMapSettings3d() {
  return mCanvas3d->mapSettings();
}

void JwLayout3D::init3DMapSettings(
    const QVector<QString> &removeLayerNames,
    const QVector<QString> &removeLayerPrefixes) {
  QgsSettings settings;
  // 创建 3D 地图设置
  // auto mapSettings3d = std::make_shared<Qgs3DMapSettings>();
  mMapSettings3d = new Qgs3DMapSettings();
  mMapSettings3d->setCrs(mProject->crs());
  // 过滤图层
  filterMapLayers(removeLayerNames, removeLayerPrefixes, mMapSettings3d);

  auto flatTerrain = new QgsFlatTerrainGenerator();
#if _QGIS_VERSION_INT >= 34100
  flatTerrain->setCrs(mMapSettings3d->crs(), mProject->transformContext());
#else
  flatTerrain->setCrs(mMapSettings3d->crs());
#endif
  mMapSettings3d->setTerrainGenerator(flatTerrain);
  // mapSettings3d->setTerrainElevationOffset(
  // project->elevationProperties()->terrainProvider()->offset() );
  //    QgsAbstractTerrainSettings *terrainSettings =
  //    QgsFlatTerrainSettings::create();
  //    terrainSettings->setElevationOffset(mProject->elevationProperties()->terrainProvider()->offset());
  //    mMapSettings3d->setTerrainSettings(terrainSettings);
  mMapSettings3d->setTerrainElevationOffset(
      mProject->elevationProperties()->terrainProvider()->offset());
  // mapSettings3d->setBackgroundColor(QColor("#ffffff"));
  spdlog::debug("filtered map layers");
  const QgsReferencedRectangle projectExtent =
      mProject->viewSettings()->fullExtent();
  const QgsRectangle fullExtent = Qgs3DUtils::tryReprojectExtent2D(
      projectExtent, projectExtent.crs(), mMapSettings3d->crs(),
      mProject->transformContext());
  spdlog::debug("get3DMapSettings fullExtent:");
  CameraUtil::ExtentInfo(fullExtent);
  mMapSettings3d->setOrigin(
      QgsVector3D(fullExtent.center().x(), fullExtent.center().y(), 0));
  spdlog::debug("set origin: {}",
                mMapSettings3d->origin().toString().toStdString());
  mMapSettings3d->setSelectionColor(mCanvas2d->selectionColor());
  spdlog::debug("set selection color: {}",
                mMapSettings3d->selectionColor().name().toStdString());
  mMapSettings3d->setBackgroundColor(mCanvas2d->canvasColor());
  spdlog::debug("set backgroupdColor: {}",
                mMapSettings3d->backgroundColor().name().toStdString());
  // mapSettings3d->setLayers( mCanvas2d->layers( true ) );
  mMapSettings3d->setTemporalRange(mCanvas2d->temporalRange());
  spdlog::debug("set temporal range: {}",
                mMapSettings3d->temporalRange().isEmpty());
  const Qgis::NavigationMode defaultNavMode =
      settings.enumValue(QStringLiteral("map3d/defaultNavigation"),
                         Qgis::NavigationMode::TerrainBased, QgsSettings::App);
  mMapSettings3d->setCameraNavigationMode(defaultNavMode);
  spdlog::debug("set camera navigation mode: {}",
                mMapSettings3d->cameraNavigationMode());
  mMapSettings3d->setCameraMovementSpeed(
      settings
          .value(QStringLiteral("map3d/defaultMovementSpeed"), 5,
                 QgsSettings::App)
          .toDouble());
  spdlog::debug("set camera movement speed: {}",
                mMapSettings3d->cameraMovementSpeed());
  const Qt3DRender::QCameraLens::ProjectionType defaultProjection =
      settings.enumValue(QStringLiteral("map3d/defaultProjection"),
                         Qt3DRender::QCameraLens::PerspectiveProjection,
                         QgsSettings::App);
  mMapSettings3d->setProjectionType(defaultProjection);
  spdlog::debug("set project type: {}", mMapSettings3d->projectionType());
  mMapSettings3d->setFieldOfView(
      settings
          .value(QStringLiteral("map3d/defaultFieldOfView"), 45,
                 QgsSettings::App)
          .toFloat());
  spdlog::debug("set field of view: {}", mMapSettings3d->fieldOfView());

  Qgs3DAxisSettings axis;
  axis.setMode(Qgs3DAxisSettings::Mode::Crs);
  spdlog::debug("mapSettings3d set3DAxisSettings");
  mMapSettings3d->set3DAxisSettings(axis);

  mMapSettings3d->setTransformContext(
      QgsProject::instance()->transformContext());
  spdlog::debug("set transform context");
  mMapSettings3d->setPathResolver(QgsProject::instance()->pathResolver());
  spdlog::debug("set path resolver");
  mMapSettings3d->setMapThemeCollection(
      QgsProject::instance()->mapThemeCollection());
  qDebug() << "set map theme collection:"
           << mMapSettings3d->mapThemeCollection();
  mMapSettings3d->configureTerrainFromProject(
      QgsProject::instance()->elevationProperties(), fullExtent);
  qDebug() << "configure terrain from project:"
           << mMapSettings3d->terrainGenerator();
  // scenes default to a single directional light
  QgsPointLightSettings defaultPointLight;
  const QgsRectangle &extent = fullExtent;
  QgsPointXY center = extent.center();
  defaultPointLight.setPosition(QgsVector3D(center.x(), center.y(), 1000));
  defaultPointLight.setConstantAttenuation(0);
  mMapSettings3d->setLightSources({defaultPointLight.clone()});
  //    auto directionalLightSettings =
  //    std::make_unique<QgsDirectionalLightSettings>();
  //    mapSettings3d->setLightSources( QList<QgsLightSource *>() <<
  //    directionalLightSettings );
  qDebug() << "set light sources:" << mMapSettings3d->lightSources();
  //    mapSettings3d->setOutputDpi(
  //    QGuiApplication::primaryScreen()->logicalDotsPerInch() );
  mMapSettings3d->setOutputDpi(300);
  spdlog::debug("set output dpi:", mMapSettings3d->outputDpi());
  mMapSettings3d->setRendererUsage(Qgis::RendererUsage::View);
  spdlog::debug("set renderer usage:", mMapSettings3d->rendererUsage());

  auto mapSett3d = mMapSettings3d;
  QObject::connect(mProject, &QgsProject::transformContextChanged, mapSett3d,
                   [this, &mapSett3d] {
                     mapSett3d->setTransformContext(
                         mProject->transformContext());
                   });
  spdlog::debug("connect project transform context changed");
  auto defaultColor = QColor(0xffffff);
  mMapSettings3d->setBackgroundColor(defaultColor);

  mMapSettings3d->setExtent(fullExtent);
  mCanvas3d->setRootEntity(new Qt3DCore::QEntity);
  mCanvas3d->setMapSettings(mMapSettings3d);
  if (!mCanvas3d->scene()) {
    spdlog::error("Error: Qgs3DMapScene or Root Entity is NULL!");
    return;
  }
  if (!mCanvas3d->engine()) {
    spdlog::error("Error: QgsWindow3DEngine is NULL!");
    return;
  }
  qDebug() << "Qgs3DMapScene created: " << (mCanvas3d->scene() != nullptr);
  qDebug() << "QgsWindow3DEngine created: " << (mCanvas3d->engine() != nullptr);
}

void JwLayout3D::setTestFrom2dExtent() {
  const QgsReferencedRectangle projectExtent =
      mProject->viewSettings()->fullExtent();
  mCanvas3d->setViewFrom2DExtent(projectExtent);
}

void JwLayout3D::setTest3DCanvas() {
  const QgsReferencedRectangle projectExtent =
      mProject->viewSettings()->fullExtent();
  auto mapSettings3d = mCanvas3d->mapSettings();
  const QgsRectangle fullExtent = Qgs3DUtils::tryReprojectExtent2D(
      projectExtent, projectExtent.crs(), mapSettings3d->crs(),
      mProject->transformContext());
  const QgsRectangle &extent = fullExtent;
  spdlog::debug("set3DCanvas fullExtent, width: {}, height: {}, center.x: {}, "
                "center.y: {}, xMin: {}, xMax: {}, yMin: {}, yMax: {}",
                extent.width(), extent.height(), extent.center().x(),
                extent.center().y(), extent.xMinimum(), extent.xMaximum(),
                extent.yMinimum(), extent.yMaximum());
  CameraUtil::ExtentInfo(extent);

  spdlog::debug("JwLayout3D::set3DCanvas");
  // mCanvas3d->setMapSettings(mMapSettings3d.get());   // jkg
  QgsVector3D lookAtCenterPoint = QgsVector3D(100, 500, 220.0);
  // QgsVector3D lookAtCenterPoint = QgsVector3D(-41966.3, -53916.1, -12671.4);
  // // dc
  QgsPointXY center(lookAtCenterPoint.x(), lookAtCenterPoint.y());
  auto distance =
      static_cast<float>(extent.width() / 1.2); // 根据场景范围调整相机距离
  float pitch = 38.0;                           // jkg
  float yaw = 20.0;                             // jkg
  // float pitch = 57.3;  // dc
  // float yaw = 321.0;   // dc
  mCanvas3d->setViewFromTop(center, distance, 0);
  mCanvas3d->cameraController()->setLookingAtPoint(lookAtCenterPoint, distance,
                                                   pitch, yaw);

  auto settedCenterPoint = mCanvas3d->cameraController()->lookingAtPoint();
  spdlog::info("setTest3DCanvas lookAtCenterPoint: {}:{}:{}",
               settedCenterPoint.x(), settedCenterPoint.y(),
               settedCenterPoint.z());
  spdlog::info("setTest3DCanvas pitch: {}, yaw: {}",
               mCanvas3d->cameraController()->pitch(),
               mCanvas3d->cameraController()->yaw());
}

/**
 * 设置 3D 地图相机参数
 * @param camera 地图相机参数
 * cameraLongitude 摄像机经度
 * cameraLatitude 摄像机纬度
 * cameraHeight 摄像机高度
 * cameraDirX 摄像机方向向量 X 分量
 * cameraDirY 摄像机方向向量 Y 分量
 * cameraDirZ 摄像机方向向量 Z 分量
 * cameraUpX 摄像机上方向向量 X 分量
 * cameraUpY 摄像机上方向向量 Y 分量
 * cameraUpZ 摄像机上方向向量 Z 分量
 * cameraRightX 摄像机右方向向量 X 分量
 * cameraRightY 摄像机右方向向量 Y 分量
 * cameraRightZ 摄像机右方向向量 Z 分量
 * fov 垂直视场角
 * aspectRatio 长宽比
 * nearPlane 近裁剪面
 * farPlane 远裁剪面
 * centerLatitude 锁定中心点纬度
 * centerLongitude 锁定中心点经度
 * centerHeight 锁定中心点高度
 * heading 摄像机偏航角
 * pitch 摄像机俯仰角
 * roll 摄像机翻滚角
 */
LookAtPoint *JwLayout3D::set3DCanvasCamera(
    DTOWRAPPERNS::DTOWrapper<Camera3dPosition> &camera, double default_distance,
    double max_pitch_angle, double offset_pull_pitch) {
  const QgsReferencedRectangle projectExtent =
      mProject->viewSettings()->fullExtent();
  auto mapSettings3d = mCanvas3d->mapSettings();
  const QgsRectangle fullExtent = Qgs3DUtils::tryReprojectExtent2D(
      projectExtent, projectExtent.crs(), mapSettings3d->crs(),
      mProject->transformContext());
  CameraUtil::ExtentInfo(fullExtent);
  
  // 获取布局中心点
  auto extentCenter = fullExtent.center();
  double centerX = extentCenter.x();
  double centerY = extentCenter.y();

  // 转换Cesium摄像机位置到场景CRS
  QgsPoint cameraLLA(camera->cameraLongitude, camera->cameraLatitude,
                     camera->cameraHeight);
  QgsPoint centerCesium(camera->centerLongitude, camera->centerLatitude,
                        camera->centerHeight);
  QgsCoordinateTransform llaToSceneCRS(
      QgsCoordinateReferenceSystem("EPSG:4326"), // WGS84经纬度
      mapSettings3d->crs(),                      // 场景CRS
      mProject->transformContext());

  // 转换Cesium坐标到场景坐标
  QgsPoint *cameraScene;
  QgsPoint *centerScene;
  try {
    cameraScene = transformPoint(cameraLLA, llaToSceneCRS);
    centerScene = transformPoint(centerCesium, llaToSceneCRS);
  } catch (const QgsCsException &e) {
    spdlog::error("摄像机位置坐标转换失败: {}", e.what().toStdString());
    return nullptr;
  }

  // 计算QGIS的pitch角
  double pitch = 0.0;
  try {
    pitch = std::stod(camera->pitch);
    if (pitch < 0) {
      pitch = max_pitch_angle + pitch;
    } else {
      pitch = max_pitch_angle - pitch;
    }
  } catch (const std::invalid_argument &e) {
    spdlog::error("Invalid pitch value: {}", camera->pitch->c_str());
  } catch (const std::out_of_range &e) {
    spdlog::error("Pitch value out of range: {}", camera->pitch->c_str());
  }
  
  // 获取Cesium的heading并转换为QGIS的yaw
  double yaw = 0.0;
  try {
    yaw = std::stod(camera->heading);
  } catch (const std::exception& e) {
    spdlog::error("Invalid heading value: {}", e.what());
    yaw = 0.0;
  }

  // 获取摄像机参数
  double fov = 45.0; // 默认视场角
  double aspectRatio = 1.0; // 默认长宽比
  double nearPlane = 1.0; // 默认近裁剪面
  double farPlane = 10000.0; // 默认远裁剪面

  try {
    if (camera->fov != nullptr && camera->fov != 45) fov = camera->fov;
    if (camera->aspectRatio != nullptr && camera->aspectRatio != 1) aspectRatio = camera->aspectRatio;
    if (camera->nearPlane != nullptr && camera->nearPlane != 1) nearPlane = camera->nearPlane;
    if (camera->farPlane != nullptr && camera->farPlane != 10000) farPlane = camera->farPlane;
  } catch (const std::exception& e) {
    spdlog::error("Error parsing camera parameters: {}", e.what());
  }

  // 计算摄像机到观察点的距离
  double heightDiff = camera->cameraHeight - camera->centerHeight;
  double baseDistance = heightDiff / std::sin(pitch * M_PI / 180.0);
  baseDistance *= 0.5;

  // 根据heading角度调整distance
  double distance = baseDistance;
  double angleRad = yaw * M_PI / 180.0;
  
  // 计算heading角度相对于0度的偏移量（0-90度）
  double angleOffset = std::fmod(yaw, 360.0);
  if (angleOffset < 0) angleOffset += 360.0;
  
  // 根据heading角度线性调整distance
  if (angleOffset >= 0 && angleOffset <= 90) {
    // 0度到90度：线性递增
    distance = baseDistance + (1000.0 * angleOffset / 90.0);
  } else if (angleOffset > 90 && angleOffset <= 180) {
    // 90度到180度：线性递减
    distance = baseDistance + (1000.0 * (180.0 - angleOffset) / 90.0);
  } else if (angleOffset > 180 && angleOffset <= 270) {
    // 180度到270度：线性递增
    distance = baseDistance + (1000.0 * (angleOffset - 180.0) / 90.0);
  } else if (angleOffset > 270 && angleOffset <= 360) {
    // 270度到360度：线性递减
    distance = baseDistance + (1000.0 * (360.0 - angleOffset) / 90.0);
  }

  // 计算视锥体参数
  double verticalFov = fov * M_PI / 180.0;
  double horizontalFov = 2.0 * std::atan(std::tan(verticalFov / 2.0) * aspectRatio);
  
  // 计算视锥体在近裁剪面的高度和宽度
  double nearHeight = 2.0 * nearPlane * std::tan(verticalFov / 2.0);
  double nearWidth = 2.0 * nearPlane * std::tan(horizontalFov / 2.0);

  // 计算观察点相对于布局中心的位置
  double qgisCenterX = 0.0;
  double qgisCenterY = centerScene->z();
  double qgisCenterZ = 0.0;

  // 使用简单的三角函数计算偏移量
  double offsetFactor = 0.5; // 调整因子
  double offsetX = std::sin(angleRad) * distance * offsetFactor;
  double offsetZ = std::cos(angleRad) * distance * offsetFactor;
  
  // 根据heading角度调整偏移方向
  if (yaw == 90) {
    // 90度时，向右偏移
    offsetX = distance * offsetFactor;
    offsetZ = 0;
  } else if (yaw == 270) {
    // 270度时，向左偏移
    offsetX = -distance * offsetFactor;
    offsetZ = 0;
  } else if (yaw == 180) {
    // 180度时，向后偏移
    offsetX = 0;
    offsetZ = -distance * offsetFactor;
  } else if (yaw == 0 || yaw == 360) {
    // 0度时，向前偏移
    offsetX = 0;
    offsetZ = distance * offsetFactor;
  }
  
  // 调整观察点位置
  qgisCenterX += offsetX;
  qgisCenterZ += offsetZ;

  // 创建观察点
  QgsVector3D lookAtCenterPosition(qgisCenterX, qgisCenterY, qgisCenterZ);

  spdlog::info("Camera parameters - fov: {}, aspectRatio: {}, nearPlane: {}, farPlane: {}, baseDistance: {}, adjustedDistance: {}, angleOffset: {}",
               fov, aspectRatio, nearPlane, farPlane, baseDistance, distance, angleOffset);
  spdlog::info("lookAtCenterPosition: {}:{}:{}, distance: {}, pitch: {}, yaw: {}, heightDiff: {}, offsetX: {}, offsetZ: {}",
               lookAtCenterPosition.x(), lookAtCenterPosition.y(), lookAtCenterPosition.z(), 
               distance, pitch, yaw, heightDiff, offsetX, offsetZ);

  // 设置摄像机参数
  mCanvas3d->cameraController()->setLookingAtPoint(
      lookAtCenterPosition, 
      static_cast<float>(distance),
      static_cast<float>(pitch),
      static_cast<float>(yaw));

  // 创建并返回LookAtPoint对象
  auto lookAtPoint = std::make_unique<LookAtPoint>(
      lookAtCenterPosition,
      static_cast<float>(distance),
      static_cast<float>(pitch),
      static_cast<float>(yaw));

  return lookAtPoint.release();
}

QgsPoint *
JwLayout3D::transformPoint(const QgsPoint &point,
                           const QgsCoordinateTransform &transformer) {
  QgsPointXY pointXY(point.x(), point.y());
  QgsPointXY transformedPointXY = transformer.transform(pointXY);
  auto qgsPoint = std::make_unique<QgsPoint>(transformedPointXY.x(),
                                             transformedPointXY.y(), point.z());
  return qgsPoint.release();
}

void JwLayout3D::set3DMap(QgsPrintLayout *layout,
                          const PaperSpecification &availablePaper,
                          DTOWRAPPERNS::DTOWrapper<Camera3dPosition> &camera,
                          int mapFrameWidth, const QString &mapFrameColor,
                          bool isDoubleFrame, double mapRotation,
                          double max_pitch_angle, double offset_pull_pitch) {
  QDomImplementation DomImplementation;
  QDomDocumentType documentType = DomImplementation.createDocumentType(
      QStringLiteral("qgis"), QStringLiteral("http://mrcc.com/qgis.dtd"),
      QStringLiteral("SYSTEM"));
  spdlog::debug("create QDomDocument:", documentType.name().toStdString());

  QDomDocument doc(documentType);

  QDomElement elem3DMap = doc.createElement(QStringLiteral("view"));
  elem3DMap.setAttribute(QStringLiteral("isOpen"), 1);
  spdlog::debug("create QDomElement:", elem3DMap.tagName().toStdString());

  // 创建 3D 地图项
  // mapItem3d = QgsLayoutItem3DMap::create(layout);
  // auto mapItem3d = std::make_unique<QgsLayoutItem3DMap>(layout);
  mMapItem3d = new QgsLayoutItem3DMap(layout);
  // spdlog::debug("mapItem3d setIsTemporal";
  // mapItem3d->setIsTemporal(true);
  spdlog::debug("mapItem3d");

  auto lookAtPoint =
      this->set3DCanvasCamera(camera, 1000, max_pitch_angle, offset_pull_pitch);

  QgsVector3D lookAtCenterPoint = QgsVector3D(
      lookAtPoint->lookingAtPoint().x(), lookAtPoint->lookingAtPoint().y(),
      lookAtPoint->lookingAtPoint().z());

  mCanvas3d->cameraController()->setLookingAtPoint(
      lookAtPoint->lookingAtPoint(), lookAtPoint->distance(),
      lookAtPoint->pitch(), lookAtPoint->yaw());

  auto settedCenterPoint = mCanvas3d->cameraController()->lookingAtPoint();
  spdlog::info("set3DMap lookAtCenterPoint: {}:{}:{}", settedCenterPoint.x(),
               settedCenterPoint.y(), settedCenterPoint.z());
  spdlog::info("set3DMap pitch: {}, yaw: {}",
               mCanvas3d->cameraController()->pitch(),
               mCanvas3d->cameraController()->yaw());
  mMapItem3d->setMapSettings(mMapSettings3d);

  Qgs3DAxisSettings axis;
  axis.setMode(Qgs3DAxisSettings::Mode::Crs);
  spdlog::debug("mapSettings3d set3DAxisSettings");
  mMapSettings3d->set3DAxisSettings(axis);

  QgsCameraPose cameraPose;
  cameraPose.setCenterPoint(lookAtCenterPoint);
  cameraPose.setDistanceFromCenterPoint(lookAtPoint->distance());
  cameraPose.setPitchAngle(lookAtPoint->pitch());
  cameraPose.setHeadingAngle(lookAtPoint->yaw());
  // set camera pose for layout
  mMapItem3d->setCameraPose(cameraPose);
  spdlog::info(
      "set3DMap cameraPose: {}:{}:{}, distance: {}, pitch: {}, yaw: {}",
      cameraPose.centerPoint().x(), cameraPose.centerPoint().y(),
      cameraPose.centerPoint().z(), cameraPose.distanceFromCenterPoint(),
      cameraPose.pitchAngle(), cameraPose.headingAngle());
  // 设置地图项大小
  mMapWidth = availablePaper.getPaperSize().second -
              mImageSpec["main_left_margin"].toDouble() -
              mImageSpec["main_right_margin"].toDouble();
  mMapHeight = availablePaper.getPaperSize().first -
               mImageSpec["main_top_margin"].toDouble() -
               mImageSpec["main_bottom_margin"].toDouble();
  // 设置地图项位置
  spdlog::debug("设置地图项位置: {}, {}, {}, {}",
                mImageSpec["main_left_margin"].toDouble(),
                mImageSpec["main_top_margin"].toDouble(), mMapWidth,
                mMapHeight);
  mMapItem3d->setFrameEnabled(true);
  mMapItem3d->setFrameStrokeColor(QColor(mapFrameColor));
  double frameWidthPixelMm =
      QgsUtil::d300PixelToMm(static_cast<float>(mapFrameWidth));
  if (isDoubleFrame) {
    mMapItem3d->setFrameStrokeWidth(
        QgsLayoutMeasurement(0.4, Qgis::LayoutUnit::Millimeters));
    float margin_offset = FRAME_OFFSET_COEFFICIENT;
    float width_offset = margin_offset * 2;
    QString fillColor = mImageSpec["main_double_frame_fill_color"].toString();
    qreal remarksX = mImageSpec["main_left_margin"].toDouble() - margin_offset;
    qreal remarksY = mImageSpec["main_top_margin"].toDouble() - margin_offset;
    qreal remarksWidth = mMapWidth + width_offset;
    qreal remarksHeight = mMapHeight + width_offset;
    auto outerFrame = addRect(fillColor, mapFrameColor, frameWidthPixelMm,
                              remarksX, remarksY, remarksWidth, remarksHeight);
    layout->addLayoutItem(outerFrame);
  } else {
    mMapItem3d->setFrameStrokeWidth(
        QgsLayoutMeasurement(frameWidthPixelMm, Qgis::LayoutUnit::Millimeters));
  }

  mMapItem3d->attemptSetSceneRect(
      QRectF(mImageSpec["main_left_margin"].toDouble(),
             mImageSpec["main_top_margin"].toDouble(), mMapWidth, mMapHeight));
  // QgsLayoutSize fixedSize(mapWidth, mapHeight,
  // Qgis::LayoutUnit::Millimeters); mapItem3d->attemptResize(fixedSize);
  // 设置相机视角
  //    spdlog::debug("设置相机视角";
  // QgsCameraPose cameraPose;
  // cameraPose.setDistanceFromCenterPoint(1788.7f); // 设置相机距离
  // mapItem3d->setCameraPose(cameraPose);
  // 添加地图项到布局
  spdlog::debug("add 3d map to layout");
  layout->addLayoutItem(mMapItem3d);
  spdlog::debug("add 3d map to layout done");
}

void JwLayout3D::addNorthArrow(
    QgsPrintLayout *layout, const QVariantMap &north,
    DTOWRAPPERNS::DTOWrapper<Camera3dPosition> &camera) {
  // 创建指北针图片项
  auto northArrow = std::make_unique<QgsLayoutItemPicture>(layout);
  // auto northArrow =  new QgsLayoutItemPicture(layout);

  // 设置指北针图片路径
  QString northArrowPath = "";
  if (mImageSpec.contains("north_base64")) {
    northArrowPath = QString("%1/%2").arg(mProjectDir, "/north_arrow.png");
    QString northBase64 = mImageSpec["north_base64"].toString();
    std::pair<QString, QByteArray> base64_image =
        ImageUtil::parse_base64_image(northBase64);
    QFile file(northArrowPath);
    if (file.open(QIODevice::WriteOnly)) {
      file.write(base64_image.second);
      file.close();
    }
  } else if (mImageSpec.contains("north_arrow_path")) {
    northArrowPath = QString("%1/%2").arg(
        mQgisPrefixPath, mImageSpec["north_arrow_path"].toString());
  } else {
    spdlog::error("the north arrow path is not set in config.yaml");
    return;
  }
  northArrow->setPicturePath(northArrowPath);

  QString northArrowFillColor = "#f30101";
  if (mImageSpec.contains("north_arrow_color")) {
    northArrowFillColor = mImageSpec["north_arrow_color"].toString();
  }
  northArrow->setSvgFillColor(QColor(northArrowFillColor));

  // 设置指北针大小和位置
  double northWidth = mImageSpec["north_arrow_width"].toDouble();
  double northHeight = mImageSpec["north_arrow_height"].toDouble();
  double northX =
      (mImageSpec["main_left_margin"].toDouble() + mMapWidth - northWidth);
  double northY = mImageSpec["main_top_margin"].toDouble() + 1.0;

  if (!north.isEmpty() and north.contains("position")) {
    if (north.value("position").toList().size() == 4) {
      northWidth =
          mMapWidth * north.value("position").toList().at(2).toDouble() / 100;
      northHeight =
          mMapHeight * north.value("position").toList().at(3).toDouble() / 100;
      northX =
          mImageSpec["main_left_margin"].toDouble() +
          mMapWidth * north.value("position").toList().at(0).toDouble() / 100;
      northY =
          mImageSpec["main_top_margin"].toDouble() +
          mMapHeight * north.value("position").toList().at(1).toDouble() / 100;
    }
  }

  northArrow->attemptSetSceneRect(
      QRectF(northX, northY, northWidth, northHeight));

  // 设置指北针旋转角度 （单位为度）
  // double northRotation = north.rotate if north and north.rotate else
  // self.image_spec.north_rotate
  // double northRotation = mImageSpec["north_rotate"].toDouble();
  double northRotation = std::stod(camera->heading);
  if (!north.isEmpty() and north.contains("rotate")) {
    northRotation = north.value("rotate").toDouble();
  }
  northArrow->setPictureRotation(northRotation);

  // 添加指北针到布局
  layout->addLayoutItem(northArrow.release());
}

void JwLayout3D::loadQptTemplate(const QString &qptFilePath,
                                 const QString &layoutTemplateName) {
  // 检查文件是否存在
  if (!QFile::exists(qptFilePath)) {
    spdlog::warn("QPT file does not exist: {}", qptFilePath.toStdString());
    return;
  }

  // 读取 QPT 文件内容
  QFile file(qptFilePath);
  if (!file.open(QIODevice::ReadOnly)) {
    spdlog::warn("Failed to open QPT file: {}", qptFilePath.toStdString());
    return;
  }

  QDomDocument doc;
  if (!doc.setContent(&file)) {
    spdlog::warn("Failed to parse QPT file: {}", qptFilePath.toStdString());
    file.close();
    return;
  }
  file.close();

  // 创建布局并加载模板
  auto layout = std::make_unique<QgsPrintLayout>(mProject);
  QgsReadWriteContext context;
  QList<QgsLayoutItem *> qgs_layout_items =
      layout->loadFromTemplate(doc, context);
  if (qgs_layout_items.isEmpty()) {
    spdlog::warn("Failed to load layout from QPT file: {}",
                 qptFilePath.toStdString());
    return;
  }

  // 设置布局名称
  layout->setName(layoutTemplateName);

  // 将布局添加到项目的布局管理器中
  mProject->layoutManager()->addLayout(layout.release());
  spdlog::debug("Loaded layout from QPT file: {}", qptFilePath.toStdString());
}

void JwLayout3D::updateLayoutExtent(const QString &layoutName) {
  // 获取布局管理器
  QgsLayoutManager *layoutManager = mProject->layoutManager();

  // 查找指定名称的布局
  auto layout =
      dynamic_cast<QgsPrintLayout *>(layoutManager->layoutByName(layoutName));
  if (!layout) {
    spdlog::warn("Layout not found: {}", layoutName.toStdString());
    return;
  }

  // 获取主窗口的地图画布, 更新布局中所有地图项的范围
  QList<QGraphicsItem *> graphics_items = layout->items();
  for (QGraphicsItem *graphics_item : graphics_items) {
    auto layout_item = dynamic_cast<QgsLayoutItem *>(graphics_item);
    if (layout_item) {
      if (auto map_item = dynamic_cast<QgsLayoutItemMap *>(layout_item)) {
        // map_item->setExtent(mCanvas3d->extent());
      }
    }
  }

  // 刷新布局
  layout->refresh();
  spdlog::debug("Updated extent for layout: {}", layoutName.toStdString());
}

// 获取图例尺寸
QPair<double, double>
JwLayout3D::getLegendDimensions(const QString &layoutName) {
  QgsMasterLayoutInterface *layoutInterface =
      mProject->layoutManager()->layoutByName(layoutName);

  if (layoutInterface) {
    if (layoutInterface->layoutType() ==
        QgsMasterLayoutInterface::PrintLayout) {
      auto layout = dynamic_cast<QgsPrintLayout *>(layoutInterface);
      for (QGraphicsItem *graphicsItem : layout->items()) {
        auto item = dynamic_cast<QgsLayoutItem *>(graphicsItem);
        if (item) {
          if (auto legend = dynamic_cast<QgsLayoutItemLegend *>(item)) {
            QRectF boundingRect = legend->boundingRect();
            return qMakePair(boundingRect.width(), boundingRect.height());
          }
        }
      }
    }
  }
  return qMakePair(0.0, 0.0);
}

void JwLayout3D::addPrintLayout(
    const QString &layoutType, const QString &layoutName,
    const QVariantMap &plottingWeb, const PaperSpecification &availablePaper,
    DTOWRAPPERNS::DTOWrapper<Camera3dPosition> &camera, bool writeQpt,
    double max_pitch_angle, double offset_pull_pitch) {
  auto plottingJson =
      JsonUtil::variantMapToJson(const_cast<QVariantMap &>(plottingWeb));
  spdlog::info("添加打印布局: {}", plottingJson.toJson());

  // 初始化布局
  spdlog::info("初始化3d布局");
  init3DLayout(layoutName);

  // auto layout = getLayout(mLayoutName);
  auto layout = getLayout3D();

  // 设置纸张类型和大小
  spdlog::debug("Setting page orientation and size");
  setPageOrientation(layout, availablePaper, 0);

  // 获取地图边框颜色、宽度和双边框设置
  QVariantMap layInfo = plottingWeb["layinfo"].toMap();
  QVariantMap bgPic = layInfo["bgPic"].toMap();
  QString mapFrameColor = bgPic.contains("borderColor")
                              ? bgPic["borderColor"].toString()
                              : mImageSpec["main_border_color"].toString();
  int mapFrameWidth = bgPic.contains("borderWidth")
                          ? bgPic["borderWidth"].toInt()
                          : mImageSpec["main_border_size"].toInt();
  bool mapDoubleFrame = bgPic.contains("doubleFrame")
                            ? bgPic["doubleFrame"].toBool()
                            : mImageSpec["main_double_frame"].toBool();
  double mapRotation = layInfo.contains("north")
                           ? layInfo["north"].toMap()["rotate"].toDouble()
                           : mImageSpec["north_rotate"].toDouble();

  float margin_offset_if_double_frame = 0.0f;
  if (mapDoubleFrame) {
    margin_offset_if_double_frame = FRAME_OFFSET_COEFFICIENT;
  }

  spdlog::info("mapFrameColor: {}, mapFrameWidth: {}, mapDoubleFrame: {}, "
               "mapRotation: {}",
               mapFrameColor.toStdString(), mapFrameWidth, mapDoubleFrame,
               mapRotation);
  // 设置地图
  qInfo() << "Added 3D map to layout";
  set3DMap(layout, availablePaper, camera, mapFrameWidth, mapFrameColor,
           mapDoubleFrame, mapRotation, max_pitch_angle, offset_pull_pitch);

  // 设置标题
  spdlog::info("设置标题");
  if (layInfo.contains("title") && !layInfo["title"].toMap().isEmpty()) {
    QMap<QString, QVariant> titleVariants = layInfo["title"].toMap();
    qInfo() << "设置标题:" << titleVariants["text"].toString();
    spdlog::info("设置标题: {}",
                 titleVariants["text"].toString().toStdString());
    setTitle(layout, titleVariants);
  }

  // 添加图例
  spdlog::debug("添加图例");
  QVariantMap legendInfo = layInfo["legend"].toMap();
  QString legendFrameColor = legendInfo.contains("borderColor")
                                 ? legendInfo["borderColor"].toString()
                                 : mImageSpec["legend_frame_color"].toString();
  QSet<QString> filteredLegendItems;
  if (plottingWeb.contains("legend") &&
      plottingWeb["legend"].toMap().contains("items")) {
    for (const QVariant &item :
         plottingWeb["legend"].toMap()["items"].toList()) {
      filteredLegendItems.insert(item.toString());
    }
  }

  spdlog::debug("filteredLegendItems Length:", filteredLegendItems.size());

  int legend_width = 40;
  if (layInfo.contains("legendWidth")) {
    legend_width = layInfo["legendWidth"].toInt();
  } else if (mImageSpec.contains("legend_width")) {
    legend_width = mImageSpec["legend_width"].toInt();
  }
  int legend_height = 80;
  if (layInfo.contains("legendHeight")) {
    legend_width = layInfo["legendHeight"].toInt();
  } else if (mImageSpec.contains("legend_height")) {
    legend_width = mImageSpec["legend_height"].toInt();
  }
  spdlog::debug("legend_width: {}, legend_height: {}, legendFrameColor: {}",
                legend_width, legend_height, legendFrameColor.toStdString());
  setLegend(layout, mImageSpec, legend_width, legend_height, legendFrameColor,
            filteredLegendItems);

  // 添加备注
  if (layInfo.contains("remark")) {
    QList<QVariant> remark_v_list = layInfo["remark"].toList();
    spdlog::debug("添加备注: {} 个", remark_v_list.length());
    for (const QVariant &remark : remark_v_list) {
      setRemarks(layout, remark.toMap(), false);
    }
  }

  // 添加指北针
  if (layInfo.contains("north")) {
    spdlog::debug("添加指北针");
    addNorthArrow(layout, layInfo["north"].toMap(), camera);
  }

  // 添加右侧索引标题
  if (layInfo.contains("subTitle")) {
    QMap<QString, QVariant> subTitleVariants = layInfo["subTitle"].toMap();
    spdlog::debug("添加右侧索引标题: {}",
                  subTitleVariants["text"].toString().toStdString());
    addRightSideLabel(layout, subTitleVariants, 7, 100, mapDoubleFrame,
                      margin_offset_if_double_frame);
  }

  // 添加签名
  if (plottingWeb.contains("pictureUnit") &&
      !plottingWeb["pictureUnit"].toString().isEmpty()) {
    spdlog::debug("添加签名: {}",
                  plottingWeb["pictureUnit"].toString().toStdString());
    addSignatureLabel(layout, plottingWeb["pictureUnit"].toString(),
                      mapDoubleFrame, margin_offset_if_double_frame);
  }

  // 添加箭头
  if (layInfo.contains("arrows")) {
    QList<QVariant> arrows = layInfo["arrows"].toList();
    spdlog::debug("添加箭头: {} 个", arrows.length());
    for (const QVariant &arrow : arrows) {
      QVariantMap arrowInfo = arrow.toMap();
      addArrowBasedOnFrontendParams(layout, arrowInfo["position"].toList(),
                                    arrowInfo["rotate"].toDouble());
    }
  }

  // 保存为 .qpt 文件
  if (writeQpt) {
    saveQptTemplate(layout);
  }
}

void JwLayout3D::exportLayoutToPng(const QString &layoutName,
                                   QString &outputPath) {
  double dpi = 150;
  if (mImageSpec.contains("main_dpi")) {
    dpi = mImageSpec["main_dpi"].toDouble();
  } else {
    spdlog::info("not found main_dpi in config.yaml, default 150");
  }

  // 检查目录是否存在，如果不存在则创建
  QFileInfo fileInfo(outputPath);
  QDir dir = fileInfo.dir();
  if (!dir.exists()) {
    spdlog::warn("Directory does not exist: {}", dir.path().toStdString());
    if (!dir.mkpath(".")) {
      spdlog::error("Failed to create directory: {}", dir.path().toStdString());
      return;
    }
  }

  if (!mLayout) {
    spdlog::warn("布局未找到: {}", layoutName.toStdString());
    return;
  }

  QgsLayoutPageCollection *pageCollection = mLayout->pageCollection();
  int pageCount = pageCollection->pageCount();
  bool should = pageCollection->shouldExportPage(0);

  QgsLayoutRenderContext &renderContext = mLayout->renderContext();
  QVector<qreal> scales;
  scales << 1.2;
  scales << 1.0;
  renderContext.setPredefinedScales(scales);
  QVector<qreal> predefinedScales = renderContext.predefinedScales();
  for (const auto &item : predefinedScales) {
    spdlog::debug("predefinedScales: {}", item);
  }
  // 创建布局导出器
  QgsLayoutExporter exporter(mLayout);
  // 设置导出选项
  QgsLayoutExporter::ImageExportSettings settings;
  settings.dpi = dpi;                                         // 设置DPI
  settings.flags |= QgsLayoutRenderContext::FlagAntialiasing; // 启用抗锯齿
  // 导出为PNG
  try {
    spdlog::debug("开始导出3D PNG: {}", outputPath.toStdString());
    // 导出为PNG图片
    QgsLayoutExporter::ExportResult result =
        exporter.exportToImage(outputPath, settings);
    if (result == QgsLayoutExporter::Success) {
      spdlog::info("导出成功: {}", outputPath.toStdString());
    } else {
      spdlog::error("导出失败，错误码: {}", static_cast<int>(result));
    }
  } catch (const std::exception &e) {
    spdlog::error("导出异常: {}", e.what());
  } catch (...) {
    spdlog::error("未知异常");
  }
}

void JwLayout3D::exportLayoutToPdf(const QString &layoutName,
                                   QString &outputPath) {
  double dpi = 150;
  if (mImageSpec.contains("main_dpi")) {
    dpi = mImageSpec["main_dpi"].toDouble();
  } else {
    spdlog::info("not found main_dpi in config.yaml, default 150");
  }

  // 检查目录是否存在，如果不存在则创建
  QFileInfo fileInfo(outputPath);
  QDir dir = fileInfo.dir();
  if (!dir.exists()) {
    spdlog::warn("Directory does not exist: {}", dir.path().toStdString());
    if (!dir.mkpath(".")) {
      spdlog::error("Failed to create directory: {}", dir.path().toStdString());
      return;
    }
  }

  if (!mLayout) {
    spdlog::warn("布局未找到: {}", layoutName.toStdString());
    return;
  }

  QgsLayoutPageCollection *pageCollection = mLayout->pageCollection();
  int pageCount = pageCollection->pageCount();
  bool should = pageCollection->shouldExportPage(0);

  QgsLayoutRenderContext &renderContext = mLayout->renderContext();
  QVector<qreal> scales;
  scales << 1.2;
  scales << 1.0;
  renderContext.setPredefinedScales(scales);
  QVector<qreal> predefinedScales = renderContext.predefinedScales();
  for (const auto &item : predefinedScales) {
    spdlog::debug("predefinedScales: {}", item);
  }
  // 创建布局导出器
  QgsLayoutExporter exporter(mLayout);
  // 设置导出选项
  QgsLayoutExporter::PdfExportSettings settings;
  settings.dpi = dpi;                                         // 设置DPI
  settings.flags |= QgsLayoutRenderContext::FlagAntialiasing; // 启用抗锯齿
  // 导出为PDF
  try {
    spdlog::debug("开始导出3D PDF: {}", outputPath.toStdString());
    QgsLayoutExporter::ExportResult result =
        exporter.exportToPdf(outputPath, settings);
    if (result == QgsLayoutExporter::Success) {
      spdlog::info("导出成功: {}", outputPath.toStdString());
    } else {
      spdlog::error("导出失败，错误码: {}", static_cast<int>(result));
    }
  } catch (const std::exception &e) {
    spdlog::error("导出异常: {}", e.what());
  } catch (...) {
    spdlog::error("未知异常");
  }
}

void JwLayout3D::exportLayoutToSvg(const QString &layoutName,
                                   QString &outputPath) {
  double dpi = 150;
  if (mImageSpec.contains("main_dpi")) {
    dpi = mImageSpec["main_dpi"].toDouble();
  } else {
    spdlog::info("not found main_dpi in config.yaml, default 150");
  }

  // 检查目录是否存在，如果不存在则创建
  QFileInfo fileInfo(outputPath);
  QDir dir = fileInfo.dir();
  if (!dir.exists()) {
    spdlog::warn("Directory does not exist: {}", dir.path().toStdString());
    if (!dir.mkpath(".")) {
      spdlog::error("Failed to create directory: {}", dir.path().toStdString());
      return;
    }
  }

  if (!mLayout) {
    spdlog::warn("布局未找到: {}", layoutName.toStdString());
    return;
  }

  QgsLayoutPageCollection *pageCollection = mLayout->pageCollection();
  int pageCount = pageCollection->pageCount();
  bool should = pageCollection->shouldExportPage(0);

  QgsLayoutRenderContext &renderContext = mLayout->renderContext();
  QVector<qreal> scales;
  scales << 1.2;
  scales << 1.0;
  renderContext.setPredefinedScales(scales);
  QVector<qreal> predefinedScales = renderContext.predefinedScales();
  for (const auto &item : predefinedScales) {
    spdlog::debug("predefinedScales: {}", item);
  }
  // 创建布局导出器
  QgsLayoutExporter exporter(mLayout);
  // 设置导出选项
  QgsLayoutExporter::SvgExportSettings settings;
  settings.dpi = dpi;                                         // 设置DPI
  settings.flags |= QgsLayoutRenderContext::FlagAntialiasing; // 启用抗锯齿

  // 导出为SVG图片
  try {
    spdlog::debug("开始导出3D SVG: {}", outputPath.toStdString());
    QgsLayoutExporter::ExportResult result =
        exporter.exportToSvg(outputPath, settings);
    if (result == QgsLayoutExporter::Success) {
      spdlog::info("导出成功: {}", outputPath.toStdString());
    } else {
      spdlog::error("导出失败，错误码: {}", static_cast<int>(result));
    }
  } catch (const std::exception &e) {
    spdlog::error("导出异常: {}", e.what());
  } catch (...) {
    spdlog::error("未知异常");
  }
}

void JwLayout3D::destroy3DCanvas() {
  try {
    mCanvas3d->destroy();
    // mCanvas3d->close();
  } catch (const std::exception &e) {
    spdlog::error("Error closing 3D canvas: {}", e.what());
  }
}

QgsLayoutItemShape *JwLayout3D::addRect(QString &fillColor,
                                        const QString &borderColor,
                                        double borderWidth, qreal remarksX,
                                        qreal remarksY, qreal remarksWidth,
                                        qreal remarksHeight) {
  auto layout = getLayout(mLayoutName);
  auto rectBg = std::make_unique<QgsLayoutItemShape>(layout);
  rectBg->setShapeType(QgsLayoutItemShape::Rectangle);
  auto symbol = std::make_unique<QgsFillSymbol>();
  symbol->setColor(QColor(fillColor));
  if (auto *symbolLayer =
          dynamic_cast<QgsSimpleFillSymbolLayer *>(symbol->symbolLayer(0))) {
    symbolLayer->setStrokeColor(QColor(borderColor));
    symbolLayer->setStrokeWidth(borderWidth);
  }
  rectBg->setSymbol(symbol.release());
  rectBg->setReferencePoint(QgsLayoutItem::ReferencePoint::LowerLeft);
  spdlog::debug(
      "remarksX: {}, remarksY: {}, remarksWidth: {}, remarksHeight: {}",
      remarksX, remarksY, remarksWidth, remarksHeight);
  rectBg->attemptSetSceneRect(
      QRectF(remarksX, remarksY, remarksWidth, remarksHeight));
  rectBg->setZValue(0);
  return rectBg.release();
}
