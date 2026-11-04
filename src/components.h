#pragma once

#include <QLabel>
#include <QFrame>
#include <QPainter>
#include <QPushButton>

namespace gui {

class TitleLabel : public QLabel {
    Q_OBJECT

public:
    explicit TitleLabel(const QString& text, QWidget* parent = nullptr)
        : QLabel(text, parent)
    {
        setStyleSheet("padding-top: 10px;");

        QFont f = font();
        f.setPointSize(10);
        f.setWeight(QFont::Bold);
        setFont(f);
    }
};

class SplitLine : public QFrame {
    Q_OBJECT

public:
    explicit SplitLine(QWidget* parent = nullptr)
        : QFrame(parent)
    {
        setFixedHeight(17);
        setStyleSheet("border: none; background-color: transparent;");
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setPen(QColor("#E0E0E0"));
        int y = height() / 2;
        p.drawLine(0, y, width(), y);
    }
};

}
