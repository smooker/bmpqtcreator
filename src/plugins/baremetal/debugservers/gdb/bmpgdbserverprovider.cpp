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

#include "bmpgdbserverprovider.h"

#include <baremetal/baremetalconstants.h>
#include <baremetal/debugserverprovidermanager.h>

#include <coreplugin/variablechooser.h>

#include <utils/fileutils.h>
#include <utils/pathchooser.h>
#include <utils/qtcassert.h>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QSpinBox>
#include <QFileInfo>
#include <QDir>

using namespace Utils;

namespace BareMetal {
namespace Internal {

const char executableFileKeyC[] = "BareMetal.BMPGdbServerProvider.ExecutableFile";
const char verboseLevelKeyC[] = "BareMetal.BMPGdbServerProvider.VerboseLevel";
const char deviceScriptC[] = "BareMetal.BMPGdbServerProvider.DeviceScript";
const char interfaceTypeC[] = "BareMetal.BMPGdbServerProvider.InterfaceType";
const char interfaceResetOnConnectC[] = "BareMetal.BMPGdbServerProvider.interfaceResetOnConnect";
const char interfaceSpeedC[] = "BareMetal.BMPGdbServerProvider.InterfaceSpeed";
const char interfaceExplicidDeviceC[] = "BareMetal.BMPGdbServerProvider.InterfaceExplicidDevice";
const char targetNameC[] = "BareMetal.BMPGdbServerProvider.TargetName";
const char targetDisableStackC[] = "BareMetal.BMPGdbServerProvider.TargetDisableStack";
const char gdbShutDownAfterDisconnectC[] = "BareMetal.BMPGdbServerProvider.GdbShutDownAfterDisconnect";
const char gdbNotUseCacheC[] = "BareMetal.BMPGdbServerProvider.GdbNotUseCache";

// BMPGdbServerProvider

BMPGdbServerProvider::BMPGdbServerProvider()
    : GdbServerProvider(Constants::GDBSERVER_BMP_PROVIDER_ID)
{
    setInitCommands(defaultInitCommands());
    setResetCommands(defaultResetCommands());
    setChannel("/dev/ttyBmpGdb", 0);
    setSettingsKeyBase("BareMetal.BMPGdbServerProvider");
    setTypeDisplayName(GdbServerProvider::tr("Black Magic Probe"));
    setConfigurationWidgetCreator([this] { return new BMPGdbServerProviderConfigWidget(this); });
}

QString BMPGdbServerProvider::defaultInitCommands()
{
    return {
        "set mem inaccessible-by-default off\n"
//        "target extended-remote /dev/ttyBmpGdb\n"
        "set remote hardware-breakpoint-limit 6\n"
        "set remote hardware-watchpoint-limit 4\n"
        "monitor swdp_scan\n"
        "attach 1\n"
//        "stop\n"
        "monitor reset halt\n"
   };
}

QString BMPGdbServerProvider::defaultResetCommands()
{
    return {"monitor reset halt\n"};
}

QSet<GdbServerProvider::StartupMode>
BMPGdbServerProvider::supportedStartupModes() const
{
    return {StartupOnPipe};
}

QString BMPGdbServerProvider::channelString() const
{
    switch (startupMode()) {
    case StartupOnNetwork:
        // Just return as "host:port" form.
        return GdbServerProvider::channelString();
    default:
        return {};
    }
}

CommandLine BMPGdbServerProvider::command() const
{
    CommandLine cmd{m_executableFile, {}};

    return cmd;
}

bool BMPGdbServerProvider::isValid() const
{
    return true;  //smooker fixme
    if (!GdbServerProvider::isValid())
        return false;

    switch (startupMode()) {
    case StartupOnPipe:
        return !m_executableFile.isEmpty();
    default:
        return false;
    }
}

QVariantMap BMPGdbServerProvider::toMap() const
{
    QVariantMap data = GdbServerProvider::toMap();
    data.insert(executableFileKeyC, m_executableFile.toVariant());

    return data;
}

bool BMPGdbServerProvider::fromMap(const QVariantMap &data)
{
    if (!GdbServerProvider::fromMap(data))
        return false;

    m_executableFile = FilePath::fromVariant(data.value(executableFileKeyC));

    return true;
}

bool BMPGdbServerProvider::operator==(const IDebugServerProvider &other) const
{
    if (!GdbServerProvider::operator==(other))
        return false;

    const auto p = static_cast<const BMPGdbServerProvider *>(&other);
    return m_executableFile == p->m_executableFile;
}

// BMPGdbServerProviderFactory

BMPGdbServerProviderFactory::BMPGdbServerProviderFactory()
{
    setId(Constants::GDBSERVER_BMP_PROVIDER_ID);
    setDisplayName(GdbServerProvider::tr("BMP"));
    setCreator([] { return new BMPGdbServerProvider; });
}

// BMPGdbServerProviderConfigWidget

BMPGdbServerProviderConfigWidget::BMPGdbServerProviderConfigWidget(
        BMPGdbServerProvider *p)
    : GdbServerProviderConfigWidget(p)
{
    Q_ASSERT(p);

    m_executableFileChooser = new PathChooser;
    m_executableFileChooser->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_mainLayout->addRow(tr("Executable file:"), m_executableFileChooser);

    m_initCommandsTextEdit = new QPlainTextEdit(this);
    m_initCommandsTextEdit->setToolTip(defaultInitCommandsTooltip());
    m_mainLayout->addRow(tr("Init commands:"), m_initCommandsTextEdit);

    m_resetCommandsTextEdit = new QPlainTextEdit(this);
    m_resetCommandsTextEdit->setToolTip(defaultResetCommandsTooltip());
    m_mainLayout->addRow(tr("Reset commands:"), m_resetCommandsTextEdit);

    addErrorLabel();
    setFromProvider();

    const auto chooser = new Core::VariableChooser(this);
    chooser->addSupportedWidget(m_initCommandsTextEdit);
    chooser->addSupportedWidget(m_resetCommandsTextEdit);

    connect(m_executableFileChooser, &Utils::PathChooser::rawPathChanged,
            this, &GdbServerProviderConfigWidget::dirty);
    connect(m_initCommandsTextEdit, &QPlainTextEdit::textChanged,
            this, &GdbServerProviderConfigWidget::dirty);
    connect(m_resetCommandsTextEdit, &QPlainTextEdit::textChanged,
            this, &GdbServerProviderConfigWidget::dirty);
}

void BMPGdbServerProviderConfigWidget::setFromProvider()
{
    const auto p = static_cast<BMPGdbServerProvider *>(m_provider);
    Q_ASSERT(p);

    m_executableFileChooser->setFileName(FilePath::fromString("/home/smooker/src/stm32/other/gcc-arm-none-eabi-9-2019-q4-major/bin/arm-none-eabi-gdb-py"));

    m_initCommandsTextEdit->setPlainText(p->initCommands());
    m_resetCommandsTextEdit->setPlainText(p->resetCommands());
}


void BMPGdbServerProviderConfigWidget::apply()
{
    const auto p = static_cast<BMPGdbServerProvider *>(m_provider);
    Q_ASSERT(p);

    p->m_executableFile = m_executableFileChooser->fileName();

    p->setInitCommands(m_initCommandsTextEdit->toPlainText());
    p->setResetCommands(m_resetCommandsTextEdit->toPlainText());
    GdbServerProviderConfigWidget::apply();
}

void BMPGdbServerProviderConfigWidget::discard()
{
    setFromProvider();
    GdbServerProviderConfigWidget::discard();
}

} // namespace Internal
} // namespace BareMetal
