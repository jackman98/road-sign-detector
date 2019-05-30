#include "detector.h"
#include "include/darknet.h"
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>         // std::mutex, std::unique_lock
#include <cmath>
#include <QBuffer>
#include <QDebug>
#include <QByteArray>
#include <QTemporaryFile>
#include <QQuickImageProvider>

SignDetector::SignDetector(QObject *parent) : QObject(parent)
{
    setConfig(":/data_for_detector/road-signs-obj-tiny.cfg");
    setWeights(":/data_for_detector/road-signs-obj-tiny.weights");
    setNamesfile(":/data_for_detector/road_signs.names");

    std::string  names_file = "/tmp/names_file";
    std::string  cfg_file = "/tmp/cfg_file";
    std::string  weights_file = "/tmp/weights_file";

    QFile::copy(m_namesfile, names_file.c_str());
    QFile::copy(m_config, cfg_file.c_str());
    QFile::copy(m_weights, weights_file.c_str());

    detector.reset(new Detector(cfg_file, weights_file));

    obj_names = objects_names_from_file(names_file);

}

QString SignDetector::config() const
{
    return m_config;
}

void SignDetector::setConfig(const QString &config)
{
    m_config = config;
}

QString SignDetector::weights() const
{
    return m_weights;
}

void SignDetector::setWeights(const QString &weights)
{
    m_weights = weights;
}

void SignDetector::draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names,
                int current_det_fps, int current_cap_fps)
{
    QStringList list;

    for (auto &i : result_vec) {
        cv::Scalar color = obj_id_to_color(i.obj_id);
        cv::rectangle(mat_img, cv::Rect(i.x, i.y, i.w, i.h), color, 2);

        if (obj_names.size() > i.obj_id) {
            std::string obj_name = obj_names[i.obj_id];
            list.append(QString::fromStdString(obj_name));

            if (i.track_id > 0) obj_name += " - " + std::to_string(i.track_id);
            cv::Size const text_size = getTextSize(obj_name, cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, 2, 0);
            int max_width = (text_size.width > i.w + 2) ? text_size.width : (i.w + 2);
            max_width = std::max(max_width, (int)i.w + 2);
            //max_width = std::max(max_width, 283);
            std::string coords_3d;

            if (!std::isnan(i.z_3d)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(2) << "x:" << i.x_3d << "m y:" << i.y_3d << "m z:" << i.z_3d << "m ";
                coords_3d = ss.str();
                cv::Size const text_size_3d = getTextSize(ss.str(), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, 1, 0);
                int const max_width_3d = (text_size_3d.width > i.w + 2) ? text_size_3d.width : (i.w + 2);
                if (max_width_3d > max_width) max_width = max_width_3d;
            }

            cv::rectangle(mat_img, cv::Point2f(std::max((int)i.x - 1, 0), std::max((int)i.y - 35, 0)),
                          cv::Point2f(std::min((int)i.x + max_width, mat_img.cols - 1), std::min((int)i.y, mat_img.rows - 1)),
                          color, CV_FILLED, 8, 0);
            putText(mat_img, obj_name, cv::Point2f(i.x, i.y - 16), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(0, 0, 0), 2);
            if(!coords_3d.empty()) putText(mat_img, coords_3d, cv::Point2f(i.x, i.y-1), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.8, cv::Scalar(0, 0, 0), 1);
        }
    }

    QString detected_signs = "signs:" + list.join(",");

    emit sendDetectedSignsStr( detected_signs);

    if (current_det_fps >= 0 && current_cap_fps >= 0) {
        std::string fps_str = "FPS detection: " + std::to_string(current_det_fps) + "   FPS capture: " + std::to_string(current_cap_fps);
        putText(mat_img, fps_str, cv::Point2f(10, 20), cv::FONT_HERSHEY_COMPLEX_SMALL, 1.2, cv::Scalar(50, 255, 0), 2);
    }
}

void SignDetector::qimage_to_mat(const QImage &image, cv::OutputArray out) {

    switch(image.format()) {
    case QImage::Format_Invalid:
    {
        cv::Mat empty;
        empty.copyTo(out);
        break;
    }
    case QImage::Format_RGB32:
    {
        cv::Mat view(image.height(),image.width(),CV_8UC4,(void *)image.constBits(),image.bytesPerLine());
        view.copyTo(out);
        break;
    }
    case QImage::Format_RGB888:
    {
        cv::Mat view(image.height(),image.width(),CV_8UC3,(void *)image.constBits(),image.bytesPerLine());
        cvtColor(view, out, cv::COLOR_RGB2BGR);
        break;
    }
    default:
    {
        QImage conv = image.convertToFormat(QImage::Format_ARGB32);
        cv::Mat view(conv.height(),conv.width(),CV_8UC4,(void *)conv.constBits(),conv.bytesPerLine());
        view.copyTo(out);
        break;
    }
    }
}

void SignDetector::mat_to_qimage(cv::InputArray image, QImage &out)
{
    switch(image.type())
    {
    case CV_8UC4:
    {
        cv::Mat view(image.getMat());
        QImage view2(view.data, view.cols, view.rows, view.step[0], QImage::Format_ARGB32);
        out = view2.copy();
        break;
    }
    case CV_8UC3:
    {
        cv::Mat mat;
        cvtColor(image, mat, cv::COLOR_BGR2BGRA); //COLOR_BGR2RGB doesn't behave so use RGBA
        QImage view(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_ARGB32);
        out = view.copy();
        break;
    }
    case CV_8UC1:
    {
        cv::Mat mat;
        cvtColor(image, mat, cv::COLOR_GRAY2BGRA);
        QImage view(mat.data, mat.cols, mat.rows, mat.step[0], QImage::Format_ARGB32);
        out = view.copy();
        break;
    }
    default:
    {
        break;
    }
    }
}

void SignDetector::test_detector(QString image_str)
{
    QByteArray array;
    array.append(image_str);

    QImage image;
    image.loadFromData(QByteArray::fromBase64(array));

    try {
        cv::Mat mat_img;
        qimage_to_mat(image, mat_img);

        //        auto start = std::chrono::steady_clock::now();
        std::vector<bbox_t> result_vec = detector->detect(mat_img);
        //        auto end = std::chrono::steady_clock::now();
        //        std::chrono::duration<double> spent = end - start;
        //        std::cout << " Time: " << spent.count() << " sec \n";

        draw_boxes(mat_img, result_vec, obj_names);

        mat_to_qimage(mat_img, image);

        array.clear();
        QBuffer bu(&array);

        bu.open(QIODevice::WriteOnly);
        image.save(&bu, "JPG");

        QString imgBase64 = QString::fromLatin1(array.toBase64().data());
        emit imageRecognized(imgBase64);
    }
    catch (std::exception &e) { std::cerr << "exception: " << e.what() << "\n"; getchar(); }
    catch (...) { std::cerr << "unknown exception \n"; getchar(); }
}

QString SignDetector::namesfile() const
{
    return m_namesfile;
}

void SignDetector::setNamesfile(const QString &namesfile)
{
    m_namesfile = namesfile;
}
