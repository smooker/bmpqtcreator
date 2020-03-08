/****************************************************************************
**
** Copyright (C) 2019 Andrey Sobol <andrey.sobol.nn@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include "gdbserverprovider.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QPlainTextEdit;
QT_END_NAMESPACE

namespace Utils { class PathChooser; }

namespace BareMetal {
namespace Internal {

// BMPGdbServerProvider

class BMPGdbServerProvider final : public GdbServerProvider
{
public:
    QVariantMap toMap() const final;
    bool fromMap(const QVariantMap &data) final;

    bool operator==(const IDebugServerProvider &other) const final;

    QString channelString() const final;
    Utils::CommandLine command() const final;

    QSet<StartupMode> supportedStartupModes() const final;
    bool isValid() const final;

private:
    BMPGdbServerProvider();

    static QString defaultInitCommands();
    static QString defaultResetCommands();

    Utils::FilePath m_executableFile =
        Utils::FilePath::fromString("BMP"); // server execute filename

    QString m_interfaceExplicidDevice;      // device=<usb_bus>:<usb_addr> ; Set device explicit

    QString scriptFileWoExt() const;

    friend class BMPGdbServerProviderConfigWidget;
    friend class BMPGdbServerProviderFactory;
};

// BMPGdbServerProviderFactory

class BMPGdbServerProviderFactory final
        : public IDebugServerProviderFactory
{
public:
    explicit BMPGdbServerProviderFactory();
};

// BMPGdbServerProviderConfigWidget

class BMPGdbServerProviderConfigWidget final
        : public GdbServerProviderConfigWidget
{
    Q_OBJECT

public:
    explicit BMPGdbServerProviderConfigWidget(
            BMPGdbServerProvider *provider);

private:
    void apply() final;
    void discard() final;

    void setFromProvider();

    Utils::PathChooser *m_executableFileChooser = nullptr;

    QPlainTextEdit *m_initCommandsTextEdit = nullptr;
    QPlainTextEdit *m_resetCommandsTextEdit = nullptr;
};

} // namespace Internal
} // namespace BareMetal

