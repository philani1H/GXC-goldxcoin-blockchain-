#include "MinerGUI.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <QCloseEvent>

// Forward declaration - will be implemented per miner type
class MiningThread : public QThread
{
    Q_OBJECT
    
public:
    MiningThread(const QString& address, const QString& rpcUrl, 
                 const QString& algorithm, int threads, QObject* parent = nullptr)
        : QThread(parent), minerAddress(address), rpcUrl(rpcUrl), 
          algorithm(algorithm), threadCount(threads), shouldStop(false) {}
    
    void stop() { shouldStop = true; }
    
signals:
    void hashRateUpdated(double rate);
    void totalHashesUpdated(uint64_t hashes);
    void blockFound(const QString& hash, double reward);
    void errorOccurred(const QString& error);
    void statusChanged(bool isMining);
    
protected:
    void run() override;
    
private:
    QString minerAddress;
    QString rpcUrl;
    QString algorithm;
    int threadCount;
    bool shouldStop;
};

void MiningThread::run()
{
    emit statusChanged(true);
    
    // TODO: Implement actual mining logic here
    // This is a placeholder that will be filled by each miner type
    
    while (!shouldStop) {
        // Simulate mining work
        QThread::msleep(100);
        
        // Emit updates
        static double rate = 0.0;
        static uint64_t hashes = 0;
        rate += 100.0;
        hashes += 1000;
        
        emit hashRateUpdated(rate);
        emit totalHashesUpdated(hashes);
    }
    
    emit statusChanged(false);
}

MinerGUI::MinerGUI(QWidget *parent)
    : QMainWindow(parent)
    , isMining(false)
    , miningThread(nullptr)
    , hashRate(0.0)
    , totalHashes(0)
    , blocksFound(0)
    , balance(0.0)
    , totalEarned(0.0)
{
    setWindowTitle("GXC Miner - Universal");
    setMinimumSize(800, 600);
    resize(900, 700);
    
    setupUI();
    setupConnections();
    
    statsTimer = new QTimer(this);
    connect(statsTimer, &QTimer::timeout, this, &MinerGUI::updateStats);
}

MinerGUI::~MinerGUI()
{
    if (miningThread && miningThread->isRunning()) {
        stopMining();
    }
}

void MinerGUI::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    
    // Status bar
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLabel = new QLabel("● Ready", this);
    statusLabel->setStyleSheet("font-weight: bold; color: green;");
    statusLayout->addWidget(statusLabel);
    statusLayout->addStretch();
    mainLayout->addLayout(statusLayout);
    
    // Stats row
    QHBoxLayout* statsLayout = new QHBoxLayout();
    hashRateLabel = new QLabel("Hash Rate: 0 H/s", this);
    totalHashesLabel = new QLabel("Total Hashes: 0", this);
    blocksFoundLabel = new QLabel("Blocks: 0", this);
    balanceLabel = new QLabel("Balance: 0.000000 GXC", this);
    
    statsLayout->addWidget(hashRateLabel);
    statsLayout->addWidget(totalHashesLabel);
    statsLayout->addWidget(blocksFoundLabel);
    statsLayout->addWidget(balanceLabel);
    statsLayout->addStretch();
    mainLayout->addLayout(statsLayout);
    
    // Configuration group
    QGroupBox* configGroup = new QGroupBox("Mining Configuration", this);
    QFormLayout* configLayout = new QFormLayout(configGroup);
    
    addressInput = new QLineEdit(this);
    addressInput->setPlaceholderText("Enter your GXC wallet address");
    configLayout->addRow("Wallet Address:", addressInput);
    
    rpcUrlInput = new QLineEdit("http://localhost:8545", this);
    configLayout->addRow("RPC URL:", rpcUrlInput);
    
    algorithmCombo = new QComboBox(this);
    algorithmCombo->addItems({"gxhash", "sha256", "ethash"});
    configLayout->addRow("Algorithm:", algorithmCombo);
    
    threadsSpinBox = new QSpinBox(this);
    threadsSpinBox->setMinimum(1);
    threadsSpinBox->setMaximum(64);
    threadsSpinBox->setValue(4);
    configLayout->addRow("Threads:", threadsSpinBox);
    
    mainLayout->addWidget(configGroup);
    
    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    startButton = new QPushButton("Start Mining", this);
    startButton->setStyleSheet("background-color: #10b981; color: white; font-weight: bold; padding: 8px;");
    stopButton = new QPushButton("Stop Mining", this);
    stopButton->setStyleSheet("background-color: #ef4444; color: white; font-weight: bold; padding: 8px;");
    stopButton->setEnabled(false);
    
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Progress bar
    progressBar = new QProgressBar(this);
    progressBar->setTextVisible(false);
    mainLayout->addWidget(progressBar);
    
    // Log area
    QGroupBox* logGroup = new QGroupBox("Activity Log", this);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    logText = new QTextEdit(this);
    logText->setReadOnly(true);
    logText->setFont(QFont("Consolas", 9));
    logLayout->addWidget(logText);
    mainLayout->addWidget(logGroup);
    
    mainLayout->setStretchFactor(logGroup, 1);
}

void MinerGUI::setupConnections()
{
    connect(startButton, &QPushButton::clicked, this, &MinerGUI::startMining);
    connect(stopButton, &QPushButton::clicked, this, &MinerGUI::stopMining);
}

void MinerGUI::startMining()
{
    if (addressInput->text().isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter your wallet address!");
        return;
    }
    
    if (isMining) {
        return;
    }
    
    // Create mining thread
    miningThread = new MiningThread(
        addressInput->text(),
        rpcUrlInput->text(),
        algorithmCombo->currentText(),
        threadsSpinBox->value(),
        this
    );
    
    connect(miningThread, &MiningThread::hashRateUpdated, this, [this](double rate) {
        hashRate = rate;
        hashRateLabel->setText(QString("Hash Rate: %1 H/s").arg(rate, 0, 'f', 2));
    });
    
    connect(miningThread, &MiningThread::totalHashesUpdated, this, [this](uint64_t hashes) {
        totalHashes = hashes;
        totalHashesLabel->setText(QString("Total Hashes: %1").arg(hashes));
    });
    
    connect(miningThread, &MiningThread::blockFound, this, &MinerGUI::onBlockFound);
    connect(miningThread, &MiningThread::errorOccurred, this, &MinerGUI::onError);
    connect(miningThread, &MiningThread::statusChanged, this, &MinerGUI::onMiningStatusChanged);
    connect(miningThread, &MiningThread::finished, miningThread, &MiningThread::deleteLater);
    
    miningThread->start();
    statsTimer->start(1000); // Update every second
    
    logText->append(QString("[%1] Mining started with algorithm: %2")
                   .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                   .arg(algorithmCombo->currentText()));
}

void MinerGUI::stopMining()
{
    if (!isMining || !miningThread) {
        return;
    }
    
    miningThread->stop();
    miningThread->wait();
    
    statsTimer->stop();
    
    logText->append(QString("[%1] Mining stopped")
                   .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

void MinerGUI::updateStats()
{
    updateMiningStats();
}

void MinerGUI::updateMiningStats()
{
    // Stats are updated via signals from mining thread
    // This method can be used for additional UI updates
}

void MinerGUI::onMiningStatusChanged(bool mining)
{
    isMining = mining;
    startButton->setEnabled(!mining);
    stopButton->setEnabled(mining);
    
    if (mining) {
        statusLabel->setText("● Mining");
        statusLabel->setStyleSheet("font-weight: bold; color: green;");
    } else {
        statusLabel->setText("● Stopped");
        statusLabel->setStyleSheet("font-weight: bold; color: red;");
    }
}

void MinerGUI::onBlockFound(const QString& blockHash, double reward)
{
    blocksFound++;
    totalEarned += reward;
    balance += reward;
    
    blocksFoundLabel->setText(QString("Blocks: %1").arg(blocksFound));
    balanceLabel->setText(QString("Balance: %1 GXC").arg(balance, 0, 'f', 6));
    
    logText->append(QString("[%1] 🎉 Block found! Hash: %2, Reward: %3 GXC")
                   .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                   .arg(blockHash.left(16))
                   .arg(reward, 0, 'f', 6));
    
    QMessageBox::information(this, "Block Found!", 
                            QString("Block mined successfully!\n\nReward: %1 GXC\nHash: %2")
                            .arg(reward, 0, 'f', 6)
                            .arg(blockHash.left(32)));
}

void MinerGUI::onError(const QString& error)
{
    logText->append(QString("[%1] ❌ Error: %2")
                   .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                   .arg(error));
    
    QMessageBox::critical(this, "Mining Error", error);
}

void MinerGUI::closeEvent(QCloseEvent *event)
{
    if (isMining) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Mining in Progress", 
            "Mining is currently running. Do you want to stop and exit?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            stopMining();
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

#include "MinerGUI.moc"
