// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2026 Kylie <kyliepc@proton.me>

#pragma once

#include <filesystem>
#include <map>
#include <optional>

#include <QByteArray>
#include <QString>

namespace Union
{
namespace Gtk
{

class ThemeAssets
{
public:
    std::optional<QString> addImage(const std::filesystem::path &source);
    std::optional<QString> addSymbolicImage(const std::filesystem::path &source);

    const std::map<QString, QByteArray> &files() const;

private:
    std::map<QString, QByteArray> m_files;
};
}
}
