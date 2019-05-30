#ifndef SIGN_DETECTOR_H
#define SIGN_DETECTOR_H

#include <QObject>
#include <QImage>
#include <QQmlEngine>

#include "include/yolo_v2_class.hpp"

class SignDetector : public QObject
{
    Q_OBJECT
public:
    explicit SignDetector(QObject *parent = nullptr);

    QString config() const;
    void setConfig(const QString &config);

    QString weights() const;
    void setWeights(const QString &weights);

    QString namesfile() const;
    void setNamesfile(const QString &namesfile);

    void draw_boxes(cv::Mat mat_img, std::vector<bbox_t> result_vec, std::vector<std::string> obj_names,
                    int current_det_fps = -1, int current_cap_fps = -1);

    void qimage_to_mat(const QImage& image, cv::OutputArray out);

    void mat_to_qimage(cv::InputArray image, QImage& out);

    std::vector<std::string> objects_names_from_file(std::string const filename) {
        std::ifstream file(filename);
        std::vector<std::string> file_lines;
        if (!file.is_open()) return file_lines;
        for(std::string line; getline(file, line);) file_lines.push_back(line);
        std::cout << "object names loaded \n";
        return file_lines;
    }
signals:
    void imageRecognized(QString imageData);
    void sendDetectedSignsStr(QString detectedSignsStr);

public slots:
    void test_detector(QString image_str);

private:
    QString m_config;
    QString m_weights;
    QString m_namesfile;

    std::shared_ptr<Detector> detector;
    std::vector<std::string> obj_names;
};

#endif // SIGN_DETECTOR_H
