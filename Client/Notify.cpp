#include "Notify.h"

#include <QTimer>
#include <QLabel>
#include <QFont>
#include <QPainter>
#include <QPaintEvent>
#include <QCoreApplication>
#include <QPropertyAnimation>

#include <QDebug>

namespace
{
    const QColor FILL_COLOR(66, 68, 70);
    const QColor TEXT_COLOR(188, 190, 191);

    const QFont defFont() { return QFont{"Segoe UI", 16}; }

    const int DISPLAY_TIME = 1500;
    const int FADEOUT_TIME = 750;
}

int Notify::m_count = 0;

Notify::Notify(const QString& text, QWidget* parent)
    : Notify(text, DISPLAY_TIME, defFont(), parent)
{}

Notify::Notify(const QString& text, int milliseconds, const QFont& font, QWidget* parent)
    : QWidget(parent)
    , m_milliseconds(milliseconds)
{
    setFont(font);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    setText(text, font);
    ++m_count;
}

Notify::~Notify()
{
    --m_count;
}

void Notify::setText(const QString& text, const QFont& font)
{
    m_text.setText(text);
    m_text.prepare(QTransform(), font);

    if (parentWidget())
    {
        QPoint offset(m_text.size().width() + 40, m_text.size().height() + 40);
        QPoint countOffset(0, m_text.size().height() + 40);
        setGeometry(QRect(parentWidget()->rect().bottomRight() - offset - countOffset * m_count, parentWidget()->rect().bottomRight() - countOffset * m_count));
    }
    else
        resize(m_text.size().width() + 20, 80);
}

void Notify::run()
{
    show();
    repaint();
    QTimer::singleShot(m_milliseconds, this, &Notify::fadeOut);
}

void Notify::fadeOut()
{
    QPropertyAnimation* animation = new QPropertyAnimation(this, "opacity", this);
    connect(animation, &QPropertyAnimation::finished, this, &Notify::deleteLater);

    animation->setDuration(FADEOUT_TIME);
    animation->setStartValue(1.);
    animation->setEndValue(0.);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Notify::showMessage(const QString& message, int milliseconds, const QFont& font, QWidget* parent)
{
    (new Notify(message, milliseconds, font, parent))->run();
}

void Notify::showMessage(const QString& message, QWidget* parent)
{
    showMessage(message, DISPLAY_TIME, defFont(), parent);
}

void Notify::paintEvent(QPaintEvent* event)
{
    QPainter p(this);

//    p.setOpacity(m_opacity);
    p.fillRect(event->rect(), FILL_COLOR);
    p.setPen(TEXT_COLOR);
    p.drawRect(event->rect().adjusted(0, 0, -1, -1));
    p.setFont(font());

    QSize halfSize = m_text.size().toSize() / 2;
    p.drawStaticText(rect().center() -= QPoint(halfSize.width(), halfSize.height()), m_text);
}
