#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;
class QToolButton;
class QPlainTextEdit;

namespace zen::ui {

class FindBar : public QWidget {
    Q_OBJECT
public:
    explicit FindBar(QWidget* parent = nullptr);

    void attachEditor(QPlainTextEdit* editor);

public slots:
    void showAndFocus();
    void hideBar();

private slots:
    void findNext();
    void findPrev();
    void onTextEdited(const QString& text);

private:
    void doFind(bool forward);

    QPlainTextEdit* m_editor {nullptr};
    QLineEdit*      m_query  {nullptr};
    QLabel*         m_status {nullptr};
    QToolButton*    m_prev   {nullptr};
    QToolButton*    m_next   {nullptr};
    QToolButton*    m_case   {nullptr};
    QToolButton*    m_word   {nullptr};
    QToolButton*    m_close  {nullptr};
};

} // namespace zen::ui
