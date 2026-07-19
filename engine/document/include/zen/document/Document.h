#pragma once

#include <QObject>
#include <QString>

#include <memory>

namespace zen::text { class TextBuffer; }

namespace zen::document {

class Document : public QObject {
    Q_OBJECT
public:
    explicit Document(QObject* parent = nullptr);
    ~Document() override;

    [[nodiscard]] QString text() const;
    void setText(const QString& text);

    // The text as of the last load/save. Used by the editor gutter to
    // render "changed since save" marks.
    [[nodiscard]] const QString& savedText() const noexcept { return m_savedText; }

    [[nodiscard]] QString filePath() const noexcept { return m_filePath; }
    [[nodiscard]] bool isModified() const noexcept  { return m_modified; }

    bool loadFromFile(const QString& path, QString* error = nullptr);
    bool saveToFile(const QString& path, QString* error = nullptr);

signals:
    void textChanged();
    void modifiedChanged(bool modified);
    void filePathChanged(const QString& path);
    void savedTextChanged();

private:
    void setModified(bool m);
    void setFilePath(const QString& path);

    std::unique_ptr<zen::text::TextBuffer> m_buffer;
    QString m_filePath;
    QString m_savedText;
    bool    m_modified {false};
};

} // namespace zen::document
