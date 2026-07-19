#include "zen/document/Document.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include "zen/text/TextBuffer.h"

namespace zen::document {

Document::Document(QObject* parent)
    : QObject(parent), m_buffer(std::make_unique<zen::text::TextBuffer>()) {}

Document::~Document() = default;

QString Document::text() const {
    const auto sv = m_buffer->text();
    return QString::fromUtf8(sv.data(), static_cast<qsizetype>(sv.size()));
}

void Document::setText(const QString& text) {
    if (this->text() == text) {
        return;
    }
    const QByteArray bytes = text.toUtf8();
    m_buffer->setText(std::string(bytes.constData(), static_cast<std::size_t>(bytes.size())));
    emit textChanged();
    setModified(text != m_savedText);
}

bool Document::loadFromFile(const QString& path, QString* error) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error) *error = f.errorString();
        return false;
    }
    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    const QString content = in.readAll();
    const QByteArray bytes = content.toUtf8();
    m_buffer->setText(std::string(bytes.constData(), static_cast<std::size_t>(bytes.size())));
    setFilePath(QFileInfo(path).absoluteFilePath());
    m_savedText = content;
    emit savedTextChanged();
    setModified(false);
    emit textChanged();
    return true;
}

bool Document::saveToFile(const QString& path, QString* error) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        if (error) *error = f.errorString();
        return false;
    }
    const auto sv = m_buffer->text();
    if (f.write(sv.data(), static_cast<qint64>(sv.size())) < 0) {
        if (error) *error = f.errorString();
        return false;
    }
    setFilePath(QFileInfo(path).absoluteFilePath());
    m_savedText = text();
    emit savedTextChanged();
    setModified(false);
    return true;
}

void Document::setModified(bool m) {
    if (m_modified == m) return;
    m_modified = m;
    emit modifiedChanged(m);
}

void Document::setFilePath(const QString& path) {
    if (m_filePath == path) return;
    m_filePath = path;
    emit filePathChanged(path);
}

} // namespace zen::document
