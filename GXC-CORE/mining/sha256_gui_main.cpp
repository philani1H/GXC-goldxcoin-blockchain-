#include "MinerGUI.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("GXC SHA256 Miner");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("GXC Development Team");
    
    // Apply modern dark theme
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(239, 68, 68));
    darkPalette.setColor(QPalette::Highlight, QColor(239, 68, 68));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    try {
        MinerGUI window;
        window.setWindowTitle("GXC SHA-256 Miner - ASIC Mining");
        window.show();
        
        return app.exec();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "GXC SHA256 Miner - Fatal Error",
            QString("A fatal error occurred:\n\n%1\n\nThe application will now exit.")
            .arg(e.what()));
        return 1;
    }
}
