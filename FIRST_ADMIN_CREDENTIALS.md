# First Admin Credentials

## ⚠️ IMPORTANT - SAVE THESE CREDENTIALS SECURELY ⚠️

---

## First Super Admin

**Username**: `Philani-GXC.Foundation`

**Password**: `GXC$ecure2025!Philani#Foundation@Admin`

**Role**: `super_admin`

**Admin ID**: `admin_philani_gxc_foundation`

**Permissions**: ALL (can do everything)

---

## What You Can Do

As the first super admin, you can:

1. ✅ **Login** to the admin system
2. ✅ **Create** additional admins
3. ✅ **Change** your password
4. ✅ **Update** admin permissions
5. ✅ **Deactivate/Reactivate** admins
6. ✅ **Remove** admins
7. ✅ **Review** market maker applications
8. ✅ **Verify** licenses with SEC/FCA/etc.
9. ✅ **Check** company reputation
10. ✅ **Review** financial standing
11. ✅ **Verify** technical capabilities
12. ✅ **Complete** KYC/AML checks
13. ✅ **Approve** market maker applications
14. ✅ **Reject** market maker applications
15. ✅ **View** statistics and audit logs

---

## How to Login

### Using API

```javascript
const response = await fetch('https://api.gxc.io/v1/admin/login', {
    method: 'POST',
    headers: {
        'Content-Type': 'application/json'
    },
    body: JSON.stringify({
        username: "Philani-GXC.Foundation",
        password: "GXC$ecure2025!Philani#Foundation@Admin"
    })
});

const data = await response.json();
console.log("Session Token:", data.sessionToken);
```

### Using Admin Dashboard

1. Open `examples/admin_dashboard.html` in your browser
2. Enter username: `Philani-GXC.Foundation`
3. Enter password: `GXC$ecure2025!Philani#Foundation@Admin`
4. Click "Login"

---

## Change Your Password (Recommended)

After first login, change your password:

```javascript
await fetch('https://api.gxc.io/v1/admin/change-password', {
    method: 'POST',
    headers: {
        'Authorization': 'Bearer YOUR_SESSION_TOKEN',
        'Content-Type': 'application/json'
    },
    body: JSON.stringify({
        adminId: "admin_philani_gxc_foundation",
        oldPassword: "GXC$ecure2025!Philani#Foundation@Admin",
        newPassword: "YOUR_NEW_SECURE_PASSWORD"
    })
});
```

---

## Create Additional Admins

### Create Verifier Admin

```javascript
await fetch('https://api.gxc.io/v1/admin/create', {
    method: 'POST',
    headers: {
        'Authorization': 'Bearer YOUR_SESSION_TOKEN',
        'Content-Type': 'application/json'
    },
    body: JSON.stringify({
        superAdminId: "admin_philani_gxc_foundation",
        username: "john_verifier",
        password: "SecurePassword123!",
        role: "verifier",
        permissions: [
            "view_applications",
            "verify_license",
            "check_reputation",
            "review_financial",
            "verify_technical",
            "complete_kyc_aml"
        ]
    })
});
```

### Create Reviewer Admin

```javascript
await fetch('https://api.gxc.io/v1/admin/create', {
    method: 'POST',
    headers: {
        'Authorization': 'Bearer YOUR_SESSION_TOKEN',
        'Content-Type': 'application/json'
    },
    body: JSON.stringify({
        superAdminId: "admin_philani_gxc_foundation",
        username: "jane_reviewer",
        password: "SecurePassword456!",
        role: "reviewer",
        permissions: [
            "view_applications",
            "assign_applications"
        ]
    })
});
```

---

## Admin Roles

### Super Admin (You)
- **Can do**: Everything
- **Permissions**: ALL
- **Examples**: Philani-GXC.Foundation

### Verifier
- **Can do**: Perform verification steps
- **Permissions**: 
  - view_applications
  - verify_license
  - check_reputation
  - review_financial
  - verify_technical
  - complete_kyc_aml
- **Cannot**: Approve/reject applications

### Reviewer
- **Can do**: View and assign applications
- **Permissions**:
  - view_applications
  - assign_applications
- **Cannot**: Perform verifications or approve/reject

---

## Security Best Practices

1. **Change Password Immediately**
   - Use a strong, unique password
   - Use a password manager

2. **Enable 2FA** (if available)
   - Add extra security layer

3. **Limit Admin Access**
   - Only create admins when needed
   - Use least privilege principle

4. **Monitor Audit Logs**
   - Review admin actions regularly
   - Check for suspicious activity

5. **Rotate Passwords**
   - Change passwords every 90 days
   - Never reuse passwords

---

## Troubleshooting

### Cannot Login

**Problem**: Invalid username or password

**Solution**:
- Check username is exactly: `Philani-GXC.Foundation`
- Check password is exactly: `GXC$ecure2025!Philani#Foundation@Admin`
- Password is case-sensitive
- No extra spaces

### Forgot Password

**Problem**: Cannot remember password

**Solution**:
- Contact GXC development team
- They can reset your password
- Or create a new super admin

### Session Expired

**Problem**: Session token no longer works

**Solution**:
- Login again to get new session token
- Session tokens expire after 24 hours

---

## Next Steps

1. **Login** with your credentials
2. **Change** your password
3. **Create** additional admins as needed
4. **Review** the admin dashboard (`examples/admin_dashboard.html`)
5. **Read** the API documentation (`MARKET_MAKER_ADMIN_API.md`)
6. **Start** accepting market maker applications

---

## Support

For help with the admin system:
- **Documentation**: `MARKET_MAKER_ADMIN_API.md`
- **System Overview**: `ADMIN_SYSTEM_SUMMARY.md`
- **API Reference**: `include/MarketMakerAdmin.h`

---

**⚠️ IMPORTANT: Save these credentials in a secure location! ⚠️**

**Do NOT share these credentials with anyone!**

**Change your password after first login!**
