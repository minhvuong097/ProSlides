#pragma once

#include "core/Types.h"

#include <QObject>
#include <QVariantList>
#include <QJsonObject>

namespace ps {
class ShowDocument;
}

namespace ps::templates {

struct Template {
    Id id;
    QString name;
    QString category;
    QString color;
    bool isDefault = false;
    QJsonObject settings;        // FreeShow TemplateSettings (verbatim)
    std::vector<Item> items;

    QJsonObject toJson() const;
    static Template fromJson(const QJsonObject& o);
};

// Loads FreeShow-compatible templates.json and applies templates to slides.
class TemplateLibrary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList templates READ templates NOTIFY templatesChanged)

public:
    explicit TemplateLibrary(QObject* parent = nullptr);

    void setFilePath(const QString& path) { m_filePath = path; }
    QString filePath() const { return m_filePath; }

    QVariantList templates() const { return m_model; }
    void load();
    bool applyToSlide(ShowDocument& doc, const QString& slideId, const QString& templateId);
    QString createFromSlide(ShowDocument& doc, const QString& name, const QString& slideId);

signals:
    void templatesChanged();
    void errorOccurred(const QString& error);

private:
    void rebuild();
    const Template* find(const QString& id) const;

    QString m_filePath;
    QVector<Template> m_templates;
    QVariantList m_model;
};

} // namespace ps::templates
