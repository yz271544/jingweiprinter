﻿//
// Created by etl on 2/3/25.
//

#include "ImageUtil.h"

std::pair<QString, QByteArray> ImageUtil::parse_base64_image(const QString& base64_str) {
    // 查找逗号的位置，使用 QString 的 split 函数
    QStringList parts = base64_str.split(",");
    if (parts.size() != 2) {
        throw std::runtime_error("Invalid base64 string format");
    }

    // 获取数据类型及 Base64 编码部分
    const QString& data_type = parts[0];
    // qDebug() << "BASE64_STR: " << base64_str;
    // qDebug() << "PARTS1_STR: " << parts[1];
    // QByteArray base64_data = base64_str.toUtf8();

    // 提取扩展名
    QStringView dataTypeView(data_type);
    int slashPos = dataTypeView.indexOf('/');
    int semicolonPos = dataTypeView.indexOf(';');
    if (slashPos == -1 || semicolonPos == -1 || slashPos >= semicolonPos) {
        throw std::runtime_error("Invalid data type format");
    }
    QString extension = dataTypeView.mid(slashPos + 1, semicolonPos - slashPos - 1).toString();

    // 对 Base64 数据进行解码
    //QByteArray image_data = QByteArray::fromBase64(base64_data);
    QByteArray image_data = QByteArray::fromBase64(parts[1].toUtf8());

    // 检查解码是否成功
    if (image_data.isEmpty()) {
        throw std::runtime_error("Base64 decoding failed");
    }

    return std::make_pair(extension, image_data);
}
