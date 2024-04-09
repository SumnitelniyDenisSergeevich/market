#pragma once

#include <QWidget>
#include <QStaticText>

class QLabel;

class Notify : public QWidget
{
    Q_OBJECT

public:
    explicit Notify(const QString& text, QWidget* parent = nullptr);
    explicit Notify(const QString& text, int milliseconds, const QFont& font, QWidget* parent = nullptr);
    virtual ~Notify();

public:
    void setText(const QString& text, const QFont& font);

    void run();

    void fadeOut();

public:
    static void showMessage(const QString& message, QWidget* parent = nullptr);
    static void showMessage(const QString& message, int milliseconds, const QFont& font, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QStaticText m_text;
    int m_milliseconds;

    static int m_count;
};
