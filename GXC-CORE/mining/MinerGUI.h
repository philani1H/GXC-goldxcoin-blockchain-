#ifndef MINER_GUI_H
#define MINER_GUI_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QTimer>
#include <QThread>
#include <QString>
#include <memory>

class MiningThread;

class MinerGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit MinerGUI(QWidget *parent = nullptr);
    ~MinerGUI();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void startMining();
    void stopMining();
    void updateStats();
    void onMiningStatusChanged(bool isMining);
    void onBlockFound(const QString& blockHash, double reward);
    void onError(const QString& error);

private:
    void setupUI();
    void setupConnections();
    void updateMiningStats();
    
    // UI Components
    QWidget* centralWidget;
    QLabel* statusLabel;
    QLabel* hashRateLabel;
    QLabel* totalHashesLabel;
    QLabel* blocksFoundLabel;
    QLabel* balanceLabel;
    
    QLineEdit* addressInput;
    QLineEdit* rpcUrlInput;
    QComboBox* algorithmCombo;
    QSpinBox* threadsSpinBox;
    QPushButton* startButton;
    QPushButton* stopButton;
    
    QTextEdit* logText;
    QProgressBar* progressBar;
    
    // Mining state
    bool isMining;
    MiningThread* miningThread;
    QTimer* statsTimer;
    
    // Statistics
    double hashRate;
    uint64_t totalHashes;
    uint32_t blocksFound;
    double balance;
    double totalEarned;
};

#endif // MINER_GUI_H
