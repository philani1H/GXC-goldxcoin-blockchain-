#include "MinerGUI.h"
#include <QApplication>
#include <QStyleFactory>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("GXC Miner");
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
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    app.setPalette(darkPalette);
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("GXC Universal Miner - GUI");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption rpcUrlOption(QStringList() << "rpc-url" << "rpc",
        "Blockchain RPC URL", "url", "http://localhost:8545");
    parser.addOption(rpcUrlOption);
    
    QCommandLineOption algorithmOption(QStringList() << "algorithm" << "algo",
        "Mining algorithm (gxhash, sha256, ethash)", "algorithm", "gxhash");
    parser.addOption(algorithmOption);
    
    parser.process(app);
    
    try {
        MinerGUI window;
        
        // Set RPC URL if provided
        if (parser.isSet(rpcUrlOption)) {
            // window.setRpcUrl(parser.value(rpcUrlOption)); // TODO: Add setter
        }
        
        // Set algorithm if provided
        if (parser.isSet(algorithmOption)) {
            // window.setAlgorithm(parser.value(algorithmOption)); // TODO: Add setter
        }
        
        window.show();
        
        return app.exec();
        
    } catch (const std::exception& e) {
        QMessageBox::critical(nullptr, "GXC Miner - Fatal Error",
            QString("A fatal error occurred:\n\n%1\n\nThe application will now exit.")
            .arg(e.what()));
        return 1;
    }
}
