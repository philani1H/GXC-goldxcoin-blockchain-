#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QAction>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSplashScreen>
#include <QtWidgets/QSystemTrayIcon>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QSettings>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <memory>

QT_CHARTS_USE_NAMESPACE

// Forward declarations
class Blockchain;
class NetworkNode;
class RPCServer;
class Wallet;

// Mining worker thread
class MiningWorker : public QThread {
    Q_OBJECT

public:
    MiningWorker(QObject* parent = nullptr);
    void setMiningParams(const QString& algorithm, int threads, const QString& address);
    void stopMining();

protected:
    void run() override;

signals:
    void hashRateUpdated(double hashRate);
    void blockFound(const QString& blockHash);
    void sharesSubmitted(int accepted, int rejected);

private:
    QString miningAlgorithm;
    int threadCount;
    QString minerAddress;
    bool shouldStop = false;
};

// Staking worker thread
class StakingWorker : public QThread {
    Q_OBJECT

public:
    StakingWorker(QObject* parent = nullptr);
    void setStakingParams(const QString& address, double amount);
    void stopStaking();

protected:
    void run() override;

signals:
    void stakingReward(double amount);
    void stakingStatusUpdate(const QString& status);

private:
    QString stakerAddress;
    double stakeAmount;
    bool shouldStop = false;
};

// Blockchain sync worker
class SyncWorker : public QThread {
    Q_OBJECT

public:
    SyncWorker(QObject* parent = nullptr);

protected:
    void run() override;

signals:
    void syncProgress(int percentage);
    void syncComplete();
    void newBlock(const QString& blockHash);
    void newTransaction(const QString& txHash);

private:
    bool shouldStop = false;
};

// Main wallet window
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // Wallet actions
    void createNewWallet();
    void loadExistingWallet();
    void backupWallet();
    void restoreWallet();
    void encryptWallet();
    void changePassword();
    void unlockWallet();
    void lockWallet();
    
    // Transaction actions
    void sendCoins();
    void receiveCoins();
    void viewTransactionHistory();
    void exportTransactions();
    
    // Mining actions
    void startMining();
    void stopMining();
    void configureMining();
    void viewMiningStats();
    
    // Staking actions
    void startStaking();
    void stopStaking();
    void configureStaking();
    void viewStakingRewards();
    
    // Network actions
    void connectToNetwork();
    void disconnectFromNetwork();
    void addPeer();
    void viewNetworkStats();
    
    // Settings and preferences
    void openSettings();
    void switchNetwork(); // Testnet/Mainnet
    void changeTheme();
    void exportPrivateKeys();
    void importPrivateKeys();
    
    // Updates from workers
    void onHashRateUpdated(double hashRate);
    void onBlockFound(const QString& blockHash);
    void onSyncProgress(int percentage);
    void onNewBlock(const QString& blockHash);
    void onNewTransaction(const QString& txHash);
    void onStakingReward(double amount);
    
    // System tray
    void showTrayIcon();
    void hideTrayIcon();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    
    // Periodic updates
    void updateBalances();
    void updateNetworkInfo();
    void updateMiningInfo();
    void updateCharts();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupSystemTray();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    
    // UI creation methods
    QWidget* createOverviewTab();
    QWidget* createWalletTab();
    QWidget* createMiningTab();
    QWidget* createStakingTab();
    QWidget* createNetworkTab();
    QWidget* createConsoleTab();
    QWidget* createSettingsTab();
    
    // Wallet UI components
    QWidget* createWalletOverview();
    QWidget* createTransactionList();
    QWidget* createAddressList();
    QWidget* createSendCoinsWidget();
    QWidget* createReceiveCoinsWidget();
    
    // Mining UI components
    QWidget* createMiningControls();
    QWidget* createMiningStats();
    QWidget* createMiningChart();
    
    // Staking UI components
    QWidget* createStakingControls();
    QWidget* createStakingStats();
    QWidget* createValidatorList();
    
    // Network UI components
    QWidget* createPeerList();
    QWidget* createNetworkStats();
    QWidget* createBlockExplorer();
    
    // Charts and visualization
    QChartView* createBalanceChart();
    QChartView* createHashRateChart();
    QChartView* createNetworkChart();
    QChartView* createPriceChart();
    
    // Dialogs
    void showWalletCreationDialog();
    void showWalletRestoreDialog();
    void showSendCoinsDialog();
    void showMiningConfigDialog();
    void showStakingConfigDialog();
    void showSettingsDialog();
    void showAboutDialog();
    
    // Utility methods
    void updateWalletInfo();
    void refreshTransactionHistory();
    void refreshAddressList();
    void updateMiningDisplay();
    void updateStakingDisplay();
    void updateNetworkDisplay();
    void showNotification(const QString& title, const QString& message);
    void setTheme(const QString& theme);
    
    // Blockchain integration
    void initializeBlockchain();
    void connectToRPC();
    QString executeRPCCommand(const QString& method, const QVariantList& params = QVariantList());
    
    // Main UI components
    QTabWidget* centralTabs;
    QStatusBar* statusBar;
    QSystemTrayIcon* trayIcon;
    QMenu* trayIconMenu;
    
    // Overview tab components
    QLabel* balanceLabel;
    QLabel* unconfirmedBalanceLabel;
    QLabel* stakingBalanceLabel;
    QLabel* networkStatusLabel;
    QLabel* blocksLabel;
    QLabel* connectionsLabel;
    QProgressBar* syncProgressBar;
    QChartView* balanceChartView;
    QListWidget* recentTransactionsList;
    
    // Wallet tab components
    QTableWidget* transactionTable;
    QTableWidget* addressTable;
    QLineEdit* sendAddressEdit;
    QDoubleSpinBox* sendAmountSpin;
    QLineEdit* sendDescriptionEdit;
    QPushButton* sendButton;
    QLineEdit* receiveAddressEdit;
    QDoubleSpinBox* receiveAmountSpin;
    QPushButton* generateAddressButton;
    
    // Mining tab components
    QComboBox* algorithmCombo;
    QSpinBox* threadsSpinBox;
    QLineEdit* poolAddressEdit;
    QLineEdit* workerNameEdit;
    QPushButton* startMiningButton;
    QPushButton* stopMiningButton;
    QLabel* hashRateLabel;
    QLabel* acceptedSharesLabel;
    QLabel* rejectedSharesLabel;
    QLabel* difficultyLabel;
    QChartView* hashRateChartView;
    QTableWidget* miningPoolsTable;
    
    // Staking tab components
    QDoubleSpinBox* stakeAmountSpin;
    QSpinBox* stakingDaysSpin;
    QPushButton* startStakingButton;
    QPushButton* stopStakingButton;
    QLabel* stakingStatusLabel;
    QLabel* stakingRewardsLabel;
    QLabel* validatorRankLabel;
    QTableWidget* validatorsTable;
    QTableWidget* stakingHistoryTable;
    
    // Network tab components
    QTableWidget* peersTable;
    QLabel* networkHashRateLabel;
    QLabel* blockTimeLabel;
    QLabel* memPoolLabel;
    QTableWidget* blocksTable;
    QLineEdit* blockExplorerEdit;
    QPushButton* searchBlockButton;
    QTextEdit* blockDetailsText;
    
    // Console tab components
    QTextEdit* consoleOutput;
    QLineEdit* consoleInput;
    QPushButton* executeButton;
    QPushButton* clearButton;
    
    // Settings
    QSettings* settings;
    QString currentTheme;
    bool isTestnet = false;
    
    // Workers
    MiningWorker* miningWorker;
    StakingWorker* stakingWorker;
    SyncWorker* syncWorker;
    
    // Timers
    QTimer* updateTimer;
    QTimer* chartUpdateTimer;
    QTimer* networkUpdateTimer;
    
    // Network
    QNetworkAccessManager* networkManager;
    
    // Blockchain components
    std::unique_ptr<Blockchain> blockchain;
    std::unique_ptr<NetworkNode> networkNode;
    std::unique_ptr<RPCServer> rpcServer;
    std::shared_ptr<Wallet> currentWallet;
    
    // Charts data
    QLineSeries* balanceSeries;
    QLineSeries* hashRateSeries;
    QLineSeries* networkHashSeries;
    QLineSeries* priceSeries;
    
    // Constants
    static constexpr int UPDATE_INTERVAL = 5000; // 5 seconds
    static constexpr int CHART_UPDATE_INTERVAL = 30000; // 30 seconds
    static constexpr int NETWORK_UPDATE_INTERVAL = 10000; // 10 seconds
    static constexpr int MAX_CHART_POINTS = 100;
};

// Wallet creation dialog
class WalletCreationDialog : public QDialog {
    Q_OBJECT

public:
    WalletCreationDialog(QWidget* parent = nullptr);
    
    QString getWalletName() const;
    QString getPassword() const;
    QString getMnemonic() const;
    bool isEncrypted() const;

private slots:
    void generateMnemonic();
    void validateInputs();

private:
    QLineEdit* walletNameEdit;
    QLineEdit* passwordEdit;
    QLineEdit* confirmPasswordEdit;
    QTextEdit* mnemonicEdit;
    QCheckBox* encryptCheckBox;
    QPushButton* generateButton;
    QDialogButtonBox* buttonBox;
};

// Send coins dialog
class SendCoinsDialog : public QDialog {
    Q_OBJECT

public:
    SendCoinsDialog(QWidget* parent = nullptr);
    
    QString getRecipientAddress() const;
    double getAmount() const;
    double getFee() const;
    QString getDescription() const;

private slots:
    void calculateFee();
    void validateTransaction();

private:
    QLineEdit* addressEdit;
    QDoubleSpinBox* amountSpin;
    QDoubleSpinBox* feeSpin;
    QLineEdit* descriptionEdit;
    QLabel* totalLabel;
    QDialogButtonBox* buttonBox;
};

// Mining configuration dialog
class MiningConfigDialog : public QDialog {
    Q_OBJECT

public:
    MiningConfigDialog(QWidget* parent = nullptr);
    
    QString getAlgorithm() const;
    int getThreads() const;
    QString getPoolAddress() const;
    QString getWorkerName() const;
    QString getMinerAddress() const;

private:
    QComboBox* algorithmCombo;
    QSpinBox* threadsSpinBox;
    QLineEdit* poolAddressEdit;
    QLineEdit* workerNameEdit;
    QLineEdit* minerAddressEdit;
    QDialogButtonBox* buttonBox;
};

// Settings dialog
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    SettingsDialog(QWidget* parent = nullptr);
    
    void loadSettings(QSettings* settings);
    void saveSettings(QSettings* settings);

private slots:
    void selectDataDirectory();
    void resetToDefaults();

private:
    QLineEdit* dataDirectoryEdit;
    QCheckBox* minimizeToTrayCheckBox;
    QCheckBox* startMinimizedCheckBox;
    QComboBox* themeCombo;
    QComboBox* networkCombo;
    QSpinBox* connectionsSpin;
    QLineEdit* rpcUserEdit;
    QLineEdit* rpcPasswordEdit;
    QSpinBox* rpcPortSpin;
    QDialogButtonBox* buttonBox;
};

#include "MainWindow.moc"