#include "ui/RoomCard.h"
#include <QMouseEvent>

RoomCard::RoomCard(QWidget* parent)
    : QWidget(parent)
    , m_roomId(0)
    , m_hovered(false)
{
    setupUI();
}

void RoomCard::setupUI() {
    setObjectName("roomCard");
    setFixedSize(280, 220);
    setCursor(Qt::PointingHandCursor);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_coverLabel = new QLabel(this);
    m_coverLabel->setObjectName("roomCover");
    m_coverLabel->setFixedSize(280, 158);
    m_coverLabel->setScaledContents(true);
    m_coverLabel->setAlignment(Qt::AlignCenter);

    QPixmap placeholder(280, 158);
    placeholder.fill(QColor("#0f3460"));
    m_coverLabel->setPixmap(placeholder);

    auto* coverLayout = new QHBoxLayout(m_coverLabel);
    coverLayout->setContentsMargins(8, 8, 8, 8);

    m_categoryLabel = new QLabel(m_coverLabel);
    m_categoryLabel->setObjectName("roomCategory");
    m_categoryLabel->setAlignment(Qt::AlignCenter);
    m_categoryLabel->setFixedHeight(22);
    m_categoryLabel->setMinimumWidth(40);
    m_categoryLabel->setText(QStringLiteral("游戏"));

    coverLayout->addStretch();
    coverLayout->addWidget(m_categoryLabel, 0, Qt::AlignTop | Qt::AlignRight);

    mainLayout->addWidget(m_coverLabel);

    m_infoBar = new QWidget(this);
    m_infoBar->setObjectName("roomInfoBar");
    m_infoBar->setFixedHeight(62);

    auto* infoLayout = new QHBoxLayout(m_infoBar);
    infoLayout->setContentsMargins(10, 6, 10, 6);
    infoLayout->setSpacing(8);

    m_avatarLabel = new QLabel(m_infoBar);
    m_avatarLabel->setObjectName("roomAvatar");
    m_avatarLabel->setFixedSize(36, 36);
    setAnchorAvatar(1);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(2);

    m_titleLabel = new QLabel(m_infoBar);
    m_titleLabel->setObjectName("roomTitle");
    m_titleLabel->setFixedWidth(170);
    m_titleLabel->setWordWrap(false);

    auto* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(4);

    m_anchorNameLabel = new QLabel(m_infoBar);
    m_anchorNameLabel->setObjectName("roomAnchorName");

    m_viewerCountLabel = new QLabel(m_infoBar);
    m_viewerCountLabel->setObjectName("roomViewerCount");

    bottomRow->addWidget(m_anchorNameLabel);
    bottomRow->addStretch();
    bottomRow->addWidget(m_viewerCountLabel);

    textLayout->addWidget(m_titleLabel);
    textLayout->addLayout(bottomRow);

    infoLayout->addWidget(m_avatarLabel);
    infoLayout->addLayout(textLayout, 1);

    mainLayout->addWidget(m_infoBar);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setOffset(0, 2);
    m_shadowEffect->setBlurRadius(8);
    m_shadowEffect->setColor(QColor(0, 0, 0, 40));
    setGraphicsEffect(m_shadowEffect);

    m_hoverAnimation = new QPropertyAnimation(this, "geometry");
    m_hoverAnimation->setDuration(150);
    m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void RoomCard::setRoomId(int id) { m_roomId = id; }
int RoomCard::roomId() const { return m_roomId; }

void RoomCard::setCoverPixmap(const QPixmap& pixmap) {
    m_coverLabel->setPixmap(pixmap.scaled(m_coverLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
}

void RoomCard::setAnchorAvatar(int avatarId) {
    QString svgPath = QString(":/avatars/avatar_%1.svg").arg(avatarId);
    QSvgRenderer renderer(svgPath);
    QPixmap pixmap(36, 36);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();
    m_avatarLabel->setPixmap(createRoundedPixmap(pixmap, 36));
}

void RoomCard::setAnchorName(const QString& name) {
    m_anchorNameLabel->setText(name);
}

void RoomCard::setTitle(const QString& title) {
    m_titleLabel->setText(title.length() > 16 ? title.left(16) + "..." : title);
}

void RoomCard::setViewerCount(int count) {
    QString text;
    if (count >= 10000) {
        text = QString::number(count / 10000.0, 'f', 1) + QStringLiteral("万");
    } else {
        text = QString::number(count);
    }
    m_viewerCountLabel->setText(QStringLiteral("👁 ") + text);
}

void RoomCard::setCategory(const QString& category) {
    m_categoryLabel->setText(category);
}

QSize RoomCard::sizeHint() const {
    return QSize(280, 220);
}

void RoomCard::enterEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hovered = true;
    m_shadowEffect->setOffset(0, 6);
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setColor(QColor(0, 0, 0, 80));
    update();
}

void RoomCard::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hovered = false;
    m_shadowEffect->setOffset(0, 2);
    m_shadowEffect->setBlurRadius(8);
    m_shadowEffect->setColor(QColor(0, 0, 0, 40));
    update();
}

void RoomCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_roomId);
    }
    QWidget::mousePressEvent(event);
}

void RoomCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#16213e"));
    painter.drawRoundedRect(rect(), 12, 12);
}

QPixmap RoomCard::createRoundedPixmap(const QPixmap& src, int size) const {
    QPixmap result(size, size);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, size, size, src);
    return result;
}
