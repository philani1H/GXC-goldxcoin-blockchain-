#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>
#include <QLocale>
#include <QMessageBox>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>

#include "MainWindow.h"
#include "../include/Logger.h"
#include "../include/Config.h"
#include "../include/Wallet.h"
#include "../include/Database.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("GXC Wallet");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("GXC Blockchain Wallet");
    app.setOrganizationName("GXC Development Team");
    app.setOrganizationDomain("gxc.network");
    
    // Set application icon
    app.setWindowIcon(QIcon(":/icons/gxc-wallet.png"));
    
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
    
    // Show splash screen
    QPixmap splashPix(":/images/splash.png");
    if (splashPix.isNull()) {
        // Create a simple splash screen if image not found
        splashPix = QPixmap(400, 300);
        splashPix.fill(QColor(42, 130, 218));
    }
    
    QSplashScreen splash(splashPix);
    splash.show();
    splash.showMessage("Initializing GXC Wallet...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
    
    app.processEvents();
    
    try {
        // Initialize logging
        splash.showMessage("Initializing logging system...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        Logger::initialize();
        LOG_GUI(LogLevel::INFO, "GXC Wallet starting up");
        
        // Initialize configuration
        splash.showMessage("Loading configuration...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        Config::initialize();
        
        // Initialize database
        splash.showMessage("Initializing database...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        std::string dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
        Database::initialize(dataDir + "/wallet.db");
        
        // Initialize wallet
        splash.showMessage("Loading wallet...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        Wallet::initialize(dataDir);
        
        // Load translations
        splash.showMessage("Loading translations...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        QTranslator translator;
        QString locale = QLocale::system().name();
        if (translator.load("gxc_" + locale, ":/translations")) {
            app.installTranslator(&translator);
        }
        
        // Create main window
        splash.showMessage("Starting user interface...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();
        
        MainWindow window;
        
        // Apply custom stylesheet
        QString styleSheet = R"(
            QMainWindow {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                    stop: 0 #2b5797, stop: 1 #1e3c72);
            }
            
            QTabWidget::pane {
                border: 1px solid #444;
                background: rgba(53, 53, 53, 180);
                border-radius: 5px;
            }
            
            QTabBar::tab {
                background: rgba(42, 130, 218, 150);
                color: white;
                padding: 10px 20px;
                margin-right: 2px;
                border-top-left-radius: 5px;
                border-top-right-radius: 5px;
            }
            
            QTabBar::tab:selected {
                background: rgba(42, 130, 218, 255);
                font-weight: bold;
            }
            
            QGroupBox {
                font-weight: bold;
                border: 2px solid #444;
                border-radius: 5px;
                margin-top: 1ex;
                background: rgba(25, 25, 25, 200);
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
                color: #42a5f5;
            }
            
            QPushButton {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                    stop: 0 #42a5f5, stop: 1 #1976d2);
                border: none;
                color: white;
                padding: 8px 16px;
                border-radius: 4px;
                font-weight: bold;
            }
            
            QPushButton:hover {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                    stop: 0 #64b5f6, stop: 1 #1e88e5);
            }
            
            QPushButton:pressed {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                    stop: 0 #1976d2, stop: 1 #0d47a1);
            }
            
            QLineEdit, QTextEdit, QSpinBox, QDoubleSpinBox {
                background: rgba(25, 25, 25, 200);
                border: 2px solid #444;
                border-radius: 4px;
                padding: 8px;
                color: white;
            }
            
            QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
                border-color: #42a5f5;
            }
            
            QTableWidget {
                gridline-color: #444;
                background: rgba(25, 25, 25, 200);
                alternate-background-color: rgba(53, 53, 53, 200);
            }
            
            QHeaderView::section {
                background: rgba(42, 130, 218, 200);
                color: white;
                padding: 8px;
                border: none;
                font-weight: bold;
            }
            
            QProgressBar {
                border: 2px solid #444;
                border-radius: 5px;
                text-align: center;
                background: rgba(25, 25, 25, 200);
            }
            
            QProgressBar::chunk {
                background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,
                    stop: 0 #4caf50, stop: 1 #2e7d32);
                border-radius: 3px;
            }
            
            QStatusBar {
                background: rgba(25, 25, 25, 200);
                border-top: 1px solid #444;
            }
            
            QMenuBar {
                background: rgba(53, 53, 53, 200);
                color: white;
            }
            
            QMenuBar::item:selected {
                background: rgba(42, 130, 218, 200);
            }
            
            QMenu {
                background: rgba(53, 53, 53, 240);
                color: white;
                border: 1px solid #444;
            }
            
            QMenu::item:selected {
                background: rgba(42, 130, 218, 200);
            }
        )";
        
        window.setStyleSheet(styleSheet);
        
        // Show main window
        QTimer::singleShot(2000, [&splash, &window]() {
            splash.finish(&window);
            window.show();
        });
        
        LOG_GUI(LogLevel::INFO, "GXC Wallet GUI initialized successfully");
        
        // Run application
        int result = app.exec();
        
        LOG_GUI(LogLevel::INFO, "GXC Wallet shutting down");
        
        // Cleanup
        Wallet::shutdown();
        Database::shutdown();
        Config::shutdown();
        Logger::shutdown();
        
        return result;
        
    } catch (const std::exception& e) {
        splash.hide();
        
        QMessageBox::critical(nullptr, "GXC Wallet - Fatal Error", 
            QString("A fatal error occurred during startup:\n\n%1\n\n"
                   "The application will now exit.").arg(e.what()));
        
        LOG_GUI(LogLevel::ERROR, "Fatal error during startup: " + std::string(e.what()));
        
        return 1;
    }
}