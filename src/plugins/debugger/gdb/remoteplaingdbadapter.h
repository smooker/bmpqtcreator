/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef REMOTEGDBCLIENTADAPTER_H
#define REMOTEGDBCLIENTADAPTER_H

#include "abstractplaingdbadapter.h"
#include "remotegdbprocess.h"

namespace Debugger {
namespace Internal {

class RemotePlainGdbAdapter : public AbstractPlainGdbAdapter
{
    Q_OBJECT

public:
    friend class RemoteGdbProcess;
    explicit RemotePlainGdbAdapter(GdbEngine *engine, QObject *parent = 0);
    void handleSetupDone();
    void handleSetupFailed(const QString &reason);

signals:
    void requestSetup();

private:
    void startAdapter();
    void setupInferior();
    void interruptInferior();
    void shutdownInferior();
    void shutdownAdapter();
    AbstractGdbProcess *gdbProc() { return &m_gdbProc; }
    DumperHandling dumperHandling() const { return DumperLoadedByGdbPreload; }

    virtual QByteArray execFilePath() const;
    virtual bool infoTargetNecessary() const;
    virtual QByteArray toLocalEncoding(const QString &s) const;
    virtual QString fromLocalEncoding(const QByteArray &b) const;
    void handleApplicationOutput(const QByteArray &output);

    RemoteGdbProcess m_gdbProc;
};

} // namespace Internal
} // namespace Debugger

#endif // REMOTEGDBCLIENTADAPTER_H
