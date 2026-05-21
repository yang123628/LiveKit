#include "ui/ReplayCard.h"
#include <QMouseEvent>

ReplayCard::ReplayCard(QWidget* parent)
    : QWidget(parent)
    , m_replayId(0)
    , m_hovered(false)
{
    setupUI();
}

void ReplayCard::setupUI() {
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

    m_durationLabel = new QLabel(m_coverLabel);
    m_durationLabel->setObjectName("replayDurationLabel");
    m_durationLabel->setAlignment(Qt::AlignCenter);
    m_durationLabel->setFixedHeight(22);
    m_durationLabel->setMinimumWidth(50);

    coverLayout->addStretch();
    coverLayout->addWidget(m_durationLabel, 0, Qt::AlignBottom | Qt::AlignRight);

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

    m_anchorNameLabel = new QLabel(m_infoBar);
    m_anchorNameLabel->setObjectName("roomAnchorName");

    textLayout->addWidget(m_titleLabel);
    textLayout->addWidget(m_anchorNameLabel);

    infoLayout->addWidget(m_avatarLabel);
    infoLayout->addLayout(textLayout, 1);

    mainLayout->addWidget(m_infoBar);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setOffset(0, 2);
    m_shadowEffect->setBlurRadius(8);
    m_shadowEffect->setColor(QColor(0, 0, 0, 40));
    setGraphicsEffect(m_shadowEffect);
}

void ReplayCard::setReplayId(int id) { m_replayId = id; }
int ReplayCard::replayId() const { return m_replayId; }

void ReplayCard::setPlayUrl(const QString& url) { m_playUrl = url; }
QString ReplayCard::playUrl() const { return m_playUrl; }

void ReplayCard::setAnchorAvatar(int avatarId) {
    QString svgPath = QString(":/avatars/avatar_%1.svg").arg(avatarId);
    QSvgRenderer renderer(svgPath);
    QPixmap pixmap(36, 36);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();

    QPixmap rounded(36, 36);
    rounded.fill(Qt::transparent);
    QPainter rp(&rounded);
    rp.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addEllipse(0, 0, 36, 36);
    rp.setClipPath(path);
    rp.drawPixmap(0, 0, 36, 36, pixmap);
    m_avatarLabel->setPixmap(rounded);
}

void ReplayCard::setAnchorName(const QString& name) {
    m_anchorNameLabel->setText(name);
}

void ReplayCard::setTitle(const QString& title) {
    m_titleLabel->setText(title.length() > 16 ? title.left(16) + "..." : title);
}

void ReplayCard::setDuration(const QString& duration) {
    m_durationLabel->setText(duration);
}

QSize ReplayCard::sizeHint() const {
    return QSize(280, 220);
}

void ReplayCard::enterEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hovered = true;
    m_shadowEffect->setOffset(0, 6);
    m_shadowEffect->setBlurRadius(20);
    m_shadowEffect->setColor(QColor(0, 0, 0, 80));
    update();
}

void ReplayCard::leaveEvent(QEvent* event) {
    Q_UNUSED(event);
    m_hovered = false;
    m_shadowEffect->setOffset(0, 2);
    m_shadowEffect->setBlurRadius(8);
    m_shadowEffect->setColor(QColor(0, 0, 0, 40));
    update();
}

void ReplayCard::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_replayId, m_playUrl);
    }
    QWidget::mousePressEvent(event);
}

void ReplayCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#16213e"));
    painter.drawRoundedRect(rect(), 12, 12);
}
