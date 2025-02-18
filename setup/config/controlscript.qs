/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** $QT_END_LICENSE$
**
****************************************************************************/

function Controller()
{    
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, 0)
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, 0)
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, 0)    
    if (installer.isInstaller())
    {
        installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, 0)        
    }
	
    installer.currentPageChanged.connect(onCurrentPageChanged)
}

uninstallIfHasInstalled = function() {
	if (!installer.isInstaller()) {
		return
	}
	
	// 如果已安装，就先卸载
	var registryPath = "HKEY_LOCAL_MACHINE\\SOFTWARE\\AkoolCam\\InstallPath"
	var installPath = installer.value(registryPath, "", QSettings.Registry32Format)
	if (installPath.length == 0) {
		console.log("not install")
		return
	}
	
	console.log("install path: " + installPath)
	var uninstallerPath = installPath + "uninstall.exe"
	if (!installer.fileExists(uninstallerPath)) {
		console.log("uninstall program not exist")
		return
	}
	
	QMessageBox.information("uninstallConfirm", "Tip", "Uninstall the previous version now", QMessageBox.Yes)
	
	installer.execute("\"" + uninstallerPath + "\"")
	
	installPath = installer.value(registryPath, "", QSettings.Registry32Format)
	if (installPath.length == 0) {
		console.log("uninstall successfully")
	}
	else {
		console.log("uninstall is cancelled")
		installer.interrupt()
	}
}

onCurrentPageChanged = function(page)
{
	if (page == QInstaller.TargetDirectory)
    {
		uninstallIfHasInstalled()
	
        components = installer.components("")
        for (var i=0; i<components.length; i++)
        {
            installer.selectComponent(components[i].name)
        }
    }
    else if (page == QInstaller.InstallationFinished)
    {
        if (installer.isInstaller())
        {     
            customInstall("install")
        }
    }
    else if (page == QInstaller.PerformInstallation)
    {        
        if (installer.isUninstaller())
        {            
            customInstall("uninstall")
        }
    }
}

customInstall = function(installParam)
{
    var installDir = installer.value("TargetDir")
    var shellPath = "\"" + installDir + "\\shellexec.exe" + "\""
    var customInstallerPath = "\"" + installDir + "\\custominst.exe" + "\""
    var params = [customInstallerPath, installParam]
    console.log("shell path: " + shellPath)
    console.log("custom installer path: " + customInstallerPath)
    result = installer.execute(shellPath, params)
    if (result.length == 0)
    {
        console.log("failed to run the program")
    }
    else
    {
        console.log("output: " + result[0])
        console.log("code: " + result[1])
    }
}