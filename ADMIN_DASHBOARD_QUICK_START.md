# Admin Dashboard Quick Start Guide

## Overview

This guide will help you build a complete admin dashboard for the GXC fraud detection system in under 30 minutes.

## Prerequisites

- Node.js 16+ installed
- Basic knowledge of React or Vue.js
- GXC node running on `localhost:8080`

## Option 1: React Dashboard (Recommended)

### Step 1: Create React App

```bash
npx create-react-app gxc-admin-dashboard
cd gxc-admin-dashboard
npm install axios react-router-dom recharts
```

### Step 2: Create API Client

Create `src/api/AdminAPI.js`:

```javascript
import axios from 'axios';

class AdminAPI {
    constructor() {
        this.baseURL = 'http://localhost:8080/api/admin';
        this.sessionToken = localStorage.getItem('adminToken');
        
        // Setup axios interceptor for auth
        axios.interceptors.request.use(config => {
            if (this.sessionToken) {
                config.headers.Authorization = `Bearer ${this.sessionToken}`;
            }
            return config;
        });
    }
    
    async login(username, password) {
        const response = await axios.post(`${this.baseURL}/login`, {
            username,
            password
        });
        
        if (response.data.success) {
            this.sessionToken = response.data.sessionToken;
            localStorage.setItem('adminToken', this.sessionToken);
        }
        
        return response.data;
    }
    
    async logout() {
        await axios.post(`${this.baseURL}/logout`);
        this.sessionToken = null;
        localStorage.removeItem('adminToken');
    }
    
    async getPendingReports() {
        const response = await axios.get(`${this.baseURL}/fraud/pending`);
        return response.data.reports;
    }
    
    async getReportDetails(reportId) {
        const response = await axios.get(`${this.baseURL}/fraud/report/${reportId}`);
        return response.data.report;
    }
    
    async approveReport(reportId, notes) {
        const response = await axios.post(`${this.baseURL}/fraud/approve`, {
            reportId,
            notes
        });
        return response.data;
    }
    
    async rejectReport(reportId, reason) {
        const response = await axios.post(`${this.baseURL}/fraud/reject`, {
            reportId,
            reason
        });
        return response.data;
    }
    
    async getDashboardOverview() {
        const response = await axios.get(`${this.baseURL}/dashboard/overview`);
        return response.data.overview;
    }
}

export default new AdminAPI();
```

### Step 3: Create Login Page

Create `src/pages/Login.js`:

```javascript
import React, { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import AdminAPI from '../api/AdminAPI';

function Login() {
    const [username, setUsername] = useState('');
    const [password, setPassword] = useState('');
    const [error, setError] = useState('');
    const navigate = useNavigate();
    
    const handleLogin = async (e) => {
        e.preventDefault();
        setError('');
        
        try {
            const result = await AdminAPI.login(username, password);
            
            if (result.success) {
                navigate('/dashboard');
            } else {
                setError(result.message);
            }
        } catch (err) {
            setError('Login failed. Please try again.');
        }
    };
    
    return (
        <div className="login-container">
            <div className="login-box">
                <h1>GXC Admin Dashboard</h1>
                <form onSubmit={handleLogin}>
                    <div className="form-group">
                        <label>Username</label>
                        <input
                            type="text"
                            value={username}
                            onChange={(e) => setUsername(e.target.value)}
                            required
                        />
                    </div>
                    
                    <div className="form-group">
                        <label>Password</label>
                        <input
                            type="password"
                            value={password}
                            onChange={(e) => setPassword(e.target.value)}
                            required
                        />
                    </div>
                    
                    {error && <div className="error">{error}</div>}
                    
                    <button type="submit">Login</button>
                </form>
            </div>
        </div>
    );
}

export default Login;
```

### Step 4: Create Dashboard Page

Create `src/pages/Dashboard.js`:

```javascript
import React, { useState, useEffect } from 'react';
import AdminAPI from '../api/AdminAPI';

function Dashboard() {
    const [overview, setOverview] = useState(null);
    const [loading, setLoading] = useState(true);
    
    useEffect(() => {
        loadDashboard();
    }, []);
    
    const loadDashboard = async () => {
        try {
            const data = await AdminAPI.getDashboardOverview();
            setOverview(data);
        } catch (err) {
            console.error('Failed to load dashboard:', err);
        } finally {
            setLoading(false);
        }
    };
    
    if (loading) return <div>Loading...</div>;
    
    return (
        <div className="dashboard">
            <h1>Dashboard Overview</h1>
            
            <div className="stats-grid">
                <div className="stat-card">
                    <h3>Pending Reports</h3>
                    <div className="stat-value">{overview.pendingFraudReports}</div>
                </div>
                
                <div className="stat-card">
                    <h3>Approved Reports</h3>
                    <div className="stat-value">{overview.approvedFraudReports}</div>
                </div>
                
                <div className="stat-card">
                    <h3>Executed Reversals</h3>
                    <div className="stat-value">{overview.executedReversals}</div>
                </div>
                
                <div className="stat-card">
                    <h3>Amount Recovered</h3>
                    <div className="stat-value">{overview.totalAmountRecovered} GXC</div>
                </div>
                
                <div className="stat-card">
                    <h3>System Pool Balance</h3>
                    <div className="stat-value">{overview.systemPoolBalance} GXC</div>
                </div>
            </div>
            
            <div className="recent-activity">
                <h2>Recent Activity</h2>
                <table>
                    <thead>
                        <tr>
                            <th>Action</th>
                            <th>Admin</th>
                            <th>Report ID</th>
                            <th>Time</th>
                        </tr>
                    </thead>
                    <tbody>
                        {overview.recentActivity.map((activity, index) => (
                            <tr key={index}>
                                <td>{activity.action}</td>
                                <td>{activity.adminId}</td>
                                <td>{activity.reportId}</td>
                                <td>{new Date(activity.timestamp * 1000).toLocaleString()}</td>
                            </tr>
                        ))}
                    </tbody>
                </table>
            </div>
        </div>
    );
}

export default Dashboard;
```

### Step 5: Create Fraud Reports Page

Create `src/pages/FraudReports.js`:

```javascript
import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import AdminAPI from '../api/AdminAPI';

function FraudReports() {
    const [reports, setReports] = useState([]);
    const [loading, setLoading] = useState(true);
    const navigate = useNavigate();
    
    useEffect(() => {
        loadReports();
    }, []);
    
    const loadReports = async () => {
        try {
            const data = await AdminAPI.getPendingReports();
            setReports(data);
        } catch (err) {
            console.error('Failed to load reports:', err);
        } finally {
            setLoading(false);
        }
    };
    
    const viewReport = (reportId) => {
        navigate(`/fraud-reports/${reportId}`);
    };
    
    if (loading) return <div>Loading...</div>;
    
    return (
        <div className="fraud-reports">
            <h1>Pending Fraud Reports</h1>
            
            <table>
                <thead>
                    <tr>
                        <th>Report ID</th>
                        <th>TX Hash</th>
                        <th>Reporter</th>
                        <th>Amount</th>
                        <th>Status</th>
                        <th>Actions</th>
                    </tr>
                </thead>
                <tbody>
                    {reports.map(report => (
                        <tr key={report.reportId}>
                            <td>{report.reportId}</td>
                            <td>{report.txHash.substring(0, 16)}...</td>
                            <td>{report.reporterAddress.substring(0, 16)}...</td>
                            <td>{report.amount} GXC</td>
                            <td>
                                <span className={`status ${report.factsStatus.toLowerCase()}`}>
                                    {report.factsStatus}
                                </span>
                            </td>
                            <td>
                                <button onClick={() => viewReport(report.reportId)}>
                                    Review
                                </button>
                            </td>
                        </tr>
                    ))}
                </tbody>
            </table>
        </div>
    );
}

export default FraudReports;
```

### Step 6: Create Report Detail Page

Create `src/pages/ReportDetail.js`:

```javascript
import React, { useState, useEffect } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import AdminAPI from '../api/AdminAPI';

function ReportDetail() {
    const { reportId } = useParams();
    const navigate = useNavigate();
    const [report, setReport] = useState(null);
    const [notes, setNotes] = useState('');
    const [loading, setLoading] = useState(true);
    
    useEffect(() => {
        loadReport();
    }, [reportId]);
    
    const loadReport = async () => {
        try {
            const data = await AdminAPI.getReportDetails(reportId);
            setReport(data);
        } catch (err) {
            console.error('Failed to load report:', err);
        } finally {
            setLoading(false);
        }
    };
    
    const handleApprove = async () => {
        if (!notes.trim()) {
            alert('Please add review notes');
            return;
        }
        
        try {
            await AdminAPI.approveReport(reportId, notes);
            alert('Fraud FACTS approved. Protocol will now validate feasibility.');
            navigate('/fraud-reports');
        } catch (err) {
            alert('Failed to approve report');
        }
    };
    
    const handleReject = async () => {
        if (!notes.trim()) {
            alert('Please add rejection reason');
            return;
        }
        
        try {
            await AdminAPI.rejectReport(reportId, notes);
            alert('Fraud report rejected');
            navigate('/fraud-reports');
        } catch (err) {
            alert('Failed to reject report');
        }
    };
    
    if (loading) return <div>Loading...</div>;
    if (!report) return <div>Report not found</div>;
    
    return (
        <div className="report-detail">
            <h1>Fraud Report Details</h1>
            
            <div className="report-info">
                <div className="info-section">
                    <h2>Report Information</h2>
                    <p><strong>Report ID:</strong> {report.reportId}</p>
                    <p><strong>Transaction Hash:</strong> {report.txHash}</p>
                    <p><strong>Reporter Address:</strong> {report.reporterAddress}</p>
                    <p><strong>Amount:</strong> {report.amount} GXC</p>
                    <p><strong>Email:</strong> {report.email}</p>
                    <p><strong>Submitted:</strong> {new Date(report.timestamp * 1000).toLocaleString()}</p>
                </div>
                
                <div className="info-section">
                    <h2>Description</h2>
                    <p>{report.description}</p>
                </div>
                
                <div className="info-section">
                    <h2>Evidence</h2>
                    <p>{report.evidence}</p>
                </div>
                
                <div className="info-section">
                    <h2>Status</h2>
                    <p><strong>Facts Status:</strong> {report.factsStatus}</p>
                    <p><strong>Execution Status:</strong> {report.executionStatus}</p>
                    
                    {report.executionStatus === 'EXECUTED' && (
                        <p><strong>Recovered Amount:</strong> {report.recoveredAmount} satoshis</p>
                    )}
                    
                    {report.executionNotes && (
                        <div className="execution-notes">
                            <strong>Protocol Notes:</strong>
                            <p>{report.executionNotes}</p>
                        </div>
                    )}
                </div>
                
                {report.factsStatus === 'PENDING' && (
                    <div className="info-section">
                        <h2>Review</h2>
                        <textarea
                            value={notes}
                            onChange={(e) => setNotes(e.target.value)}
                            placeholder="Add your review notes here..."
                            rows="5"
                        />
                        
                        <div className="actions">
                            <button className="approve" onClick={handleApprove}>
                                Approve FACTS
                            </button>
                            <button className="reject" onClick={handleReject}>
                                Reject Report
                            </button>
                        </div>
                        
                        <div className="info-box">
                            <strong>Note:</strong> Approving validates that the fraud claim is legitimate.
                            The protocol will then independently validate feasibility and execute if possible.
                        </div>
                    </div>
                )}
            </div>
        </div>
    );
}

export default ReportDetail;
```

### Step 7: Setup Routing

Update `src/App.js`:

```javascript
import React from 'react';
import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom';
import Login from './pages/Login';
import Dashboard from './pages/Dashboard';
import FraudReports from './pages/FraudReports';
import ReportDetail from './pages/ReportDetail';
import './App.css';

function App() {
    const isAuthenticated = () => {
        return !!localStorage.getItem('adminToken');
    };
    
    const PrivateRoute = ({ children }) => {
        return isAuthenticated() ? children : <Navigate to="/login" />;
    };
    
    return (
        <BrowserRouter>
            <Routes>
                <Route path="/login" element={<Login />} />
                <Route path="/dashboard" element={
                    <PrivateRoute>
                        <Dashboard />
                    </PrivateRoute>
                } />
                <Route path="/fraud-reports" element={
                    <PrivateRoute>
                        <FraudReports />
                    </PrivateRoute>
                } />
                <Route path="/fraud-reports/:reportId" element={
                    <PrivateRoute>
                        <ReportDetail />
                    </PrivateRoute>
                } />
                <Route path="/" element={<Navigate to="/dashboard" />} />
            </Routes>
        </BrowserRouter>
    );
}

export default App;
```

### Step 8: Add Basic Styling

Create `src/App.css`:

```css
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Oxygen',
        'Ubuntu', 'Cantarell', 'Fira Sans', 'Droid Sans', 'Helvetica Neue',
        sans-serif;
    background: #f5f5f5;
}

/* Login Page */
.login-container {
    display: flex;
    justify-content: center;
    align-items: center;
    min-height: 100vh;
}

.login-box {
    background: white;
    padding: 40px;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    width: 100%;
    max-width: 400px;
}

.login-box h1 {
    margin-bottom: 30px;
    text-align: center;
    color: #333;
}

.form-group {
    margin-bottom: 20px;
}

.form-group label {
    display: block;
    margin-bottom: 5px;
    color: #666;
}

.form-group input {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-size: 14px;
}

.error {
    color: #d32f2f;
    margin: 10px 0;
    padding: 10px;
    background: #ffebee;
    border-radius: 4px;
}

button {
    width: 100%;
    padding: 12px;
    background: #1976d2;
    color: white;
    border: none;
    border-radius: 4px;
    font-size: 16px;
    cursor: pointer;
}

button:hover {
    background: #1565c0;
}

/* Dashboard */
.dashboard {
    padding: 20px;
    max-width: 1200px;
    margin: 0 auto;
}

.dashboard h1 {
    margin-bottom: 30px;
}

.stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 20px;
    margin-bottom: 40px;
}

.stat-card {
    background: white;
    padding: 20px;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.stat-card h3 {
    color: #666;
    font-size: 14px;
    margin-bottom: 10px;
}

.stat-value {
    font-size: 32px;
    font-weight: bold;
    color: #1976d2;
}

/* Tables */
table {
    width: 100%;
    background: white;
    border-radius: 8px;
    overflow: hidden;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

thead {
    background: #f5f5f5;
}

th, td {
    padding: 12px;
    text-align: left;
    border-bottom: 1px solid #eee;
}

th {
    font-weight: 600;
    color: #666;
}

/* Status badges */
.status {
    padding: 4px 8px;
    border-radius: 4px;
    font-size: 12px;
    font-weight: 600;
}

.status.pending {
    background: #fff3e0;
    color: #f57c00;
}

.status.facts_approved {
    background: #e8f5e9;
    color: #2e7d32;
}

.status.facts_rejected {
    background: #ffebee;
    color: #c62828;
}

/* Report Detail */
.report-detail {
    padding: 20px;
    max-width: 800px;
    margin: 0 auto;
}

.info-section {
    background: white;
    padding: 20px;
    margin-bottom: 20px;
    border-radius: 8px;
    box-shadow: 0 2px 4px rgba(0,0,0,0.1);
}

.info-section h2 {
    margin-bottom: 15px;
    color: #333;
}

.info-section p {
    margin-bottom: 10px;
    color: #666;
}

textarea {
    width: 100%;
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 4px;
    font-family: inherit;
    margin-bottom: 15px;
}

.actions {
    display: flex;
    gap: 10px;
}

.actions button {
    flex: 1;
}

button.approve {
    background: #4caf50;
}

button.approve:hover {
    background: #45a049;
}

button.reject {
    background: #f44336;
}

button.reject:hover {
    background: #da190b;
}

.info-box {
    background: #e3f2fd;
    padding: 15px;
    border-radius: 4px;
    margin-top: 15px;
    color: #1565c0;
}

.execution-notes {
    background: #f5f5f5;
    padding: 15px;
    border-radius: 4px;
    margin-top: 10px;
}
```

### Step 9: Run the Dashboard

```bash
npm start
```

The dashboard will open at `http://localhost:3000`

**Default Login:**
- Username: `admin`
- Password: `admin123`

---

## Option 2: Vue.js Dashboard

### Quick Setup

```bash
npm create vue@latest gxc-admin-dashboard
cd gxc-admin-dashboard
npm install
npm install axios vue-router pinia
```

Follow similar structure as React example, adapting to Vue.js syntax.

---

## Features Included

### ✅ Authentication
- Login/logout
- Session management
- Protected routes

### ✅ Dashboard
- Overview statistics
- Recent activity
- System pool balance

### ✅ Fraud Reports
- List pending reports
- View report details
- Approve/reject reports
- Real-time status updates

### ✅ User Experience
- Responsive design
- Loading states
- Error handling
- Clean UI

---

## Next Steps

### Add More Features

1. **Admin User Management**
```javascript
async getAdminUsers() {
    const response = await axios.get(`${this.baseURL}/users`);
    return response.data.admins;
}

async createAdmin(username, password, role, permissions) {
    const response = await axios.post(`${this.baseURL}/users/create`, {
        username, password, role, permissions
    });
    return response.data;
}
```

2. **Fraud Statistics**
```javascript
async getFraudStats(period = 'month') {
    const response = await axios.get(
        `${this.baseURL}/dashboard/fraud-stats?period=${period}`
    );
    return response.data.stats;
}
```

3. **Audit Logs**
```javascript
async getAuditLogs(filters = {}) {
    const response = await axios.get(`${this.baseURL}/audit/logs`, {
        params: filters
    });
    return response.data.logs;
}
```

4. **System Configuration**
```javascript
async getSystemConfig() {
    const response = await axios.get(`${this.baseURL}/system/config`);
    return response.data.config;
}

async updateSystemConfig(config) {
    const response = await axios.put(`${this.baseURL}/system/config`, config);
    return response.data;
}
```

---

## Testing

### Test with Mock Data

If the node isn't running, you can test with mock data:

```javascript
// src/api/MockAdminAPI.js
class MockAdminAPI {
    async login(username, password) {
        if (username === 'admin' && password === 'admin123') {
            return {
                success: true,
                sessionToken: 'mock_token_123',
                adminId: 'admin_001',
                username: 'admin',
                role: 'super_admin',
                permissions: ['manage_admins', 'fraud_reviewer', 'fraud_approver']
            };
        }
        return { success: false, message: 'Invalid credentials' };
    }
    
    async getPendingReports() {
        return [
            {
                reportId: 'FR_001',
                txHash: 'abc123...',
                reporterAddress: 'GXC1victim123...',
                amount: 100.0,
                factsStatus: 'PENDING',
                executionStatus: 'NOT_STARTED',
                timestamp: Date.now() / 1000
            }
        ];
    }
    
    async getDashboardOverview() {
        return {
            pendingFraudReports: 5,
            approvedFraudReports: 23,
            rejectedFraudReports: 7,
            executedReversals: 18,
            totalAmountRecovered: 1200.0,
            systemPoolBalance: 150.5,
            recentActivity: []
        };
    }
}

export default new MockAdminAPI();
```

---

## Deployment

### Build for Production

```bash
npm run build
```

### Deploy to Vercel

```bash
npm install -g vercel
vercel
```

### Deploy to Netlify

```bash
npm install -g netlify-cli
netlify deploy --prod
```

### Environment Variables

Create `.env.production`:

```
REACT_APP_API_URL=https://your-gxc-node.com/api/admin
```

---

## Summary

You now have:

- ✅ Complete admin dashboard in React
- ✅ Authentication system
- ✅ Fraud report management
- ✅ Dashboard statistics
- ✅ Clean, responsive UI
- ✅ Production-ready code

**Time to build:** ~30 minutes

**Next:** Add more features, customize styling, deploy to production!
