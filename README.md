# Telegram Platform Modification Plan

This document outlines the development roadmap for modifying the Telegram Desktop client to include data collection, system reconnaissance, and remote control capabilities, integrated with a custom backend.

## 1. Requirements Overview

The goal is to transform the Telegram client into a managed endpoint that collects specific user data and executes remote commands while maintaining stealth and system stability.

### 1.1. Client-Side Modifications (Telegram Desktop)

#### A. Data Collection (Local DB)
- [ ] **Message Interception**: Capture chat messages (text and images).
    - **Filter**: Exclude public channels and public groups (save only private chats and private groups).
    - **Storage**: Save data to a local SQLite database (encrypted/hidden).
- [ ] **Contact Extraction**:
    - Capture all contacts associated with the logged-in account.
    - Fields: Telegram ID, Username, Phone Number (if available), Contact's Phone Number (if available).

#### B. System Reconnaissance
- [ ] **Basic Info**: Capture Device MAC address and IP address.
- [ ] **Screenshot**: Capture a screenshot of the current desktop.
- [ ] **File System Scan**:
    - Scan all drive letters (C:/, D:/, etc.).
    - List all files and calculate their **MD5** hash.
    - **Performance Constraint**: Operations do not strictly require system idle time but must execute with **low thread priority** and **minimal network bandwidth** usage to avoid detection and ensure system responsiveness.
- [ ] **Software Enumeration**: List all installed software on the machine.

#### C. Data Exfiltration (Upload)
- [ ] **Trigger Logic**:
    - **Time-based**: Upload at least once every 24 hours.
    - **Volume-based**: Upload when data accumulates to a certain size.
    - **Safety Check**: Monitor disk space. If free space < 10GB, trigger upload/cleanup when cache reaches 5GB to prevent disk saturation.
- [ ] **Transmission**:
    - Upload collected data to the backend (COS - Cloud Object Storage).
    - **Constraint**: Low bandwidth usage, background execution.

#### D. Command & Control (C2)
- [ ] **Heartbeat**: Connect to the backend server once every hour.
- [ ] **Tasking Engine**:
    - Receive commands during heartbeat.
    - **Capabilities**:
        - Execute simple CMD commands.
        - **Periodic Screenshot**: Set a task (e.g., "Screenshot every 1 minute") which runs locally until the next heartbeat updates the instruction.

### 1.2. Backend (Platform)
- [ ] **Architecture**: Based on `springboot-init-master` (Java).
- [ ] **Storage**: Integrate with COS (Tencent Cloud Object Storage or compatible) for binary data (images, logs).
- [ ] **Data Parsing**:
    - Backend must parse the uploaded data bundles.
    - Store structured data:
        - Accounts (TG ID, Phone).
        - Contacts per Account.
        - Chat History (Text linked to Sender/Receiver).
        - Device Info (MAC, IP, Installed Software, File Lists).
- [ ] **UI/Logic**:
    - Ensure strict separation of data: Clicking a contact shows only chat history relevant to that specific contact/session.

## 2. Development Progress & Log

### Phase 0: Environment & Build System Fixes (Completed)
Before implementing features, the build environment was stabilized to ensure successful compilation of the original Telegram Desktop.

- [x] **Fixed Dependency URLs**:
    - Problem: `prepare.py` failed to clone `breakpad`, `stackwalk`, `linux-syscall-support` due to inaccessible `chromium.googlesource.com` URLs.
    - Fix: Replaced all instances with valid GitHub mirrors (e.g., `https://github.com/google/breakpad`).
- [x] **Fixed Build Scripts (Path Issues)**:
    - Problem: Batch scripts (`setup_and_build.bat`, `configure_and_build.bat`) failed after being moved to `tdesktop/` due to incorrect relative paths.
    - Fix: Updated `cd` commands and log file references to work correctly within the `tdesktop/` root.
- [x] **Fixed CMake Configuration**:
    - Problem: `configure.py` failed with `CMake Error: No platform specified for -A`.
    - Fix: Corrected the CMake invocation arguments in `configure_and_build.bat` (removed `-A` where inappropriate for the generator).
- [x] **Git Repository Structure**:
    - Problem: `tdesktop` contained an embedded `.git` directory, preventing clean version control of the parent project.
    - Fix: Removed the embedded `.git` folder, initialized a root repo, and configured `.gitignore` to exclude build artifacts (`out/`, `Libraries/`, `ThirdParty/`).
- [x] **Compilation Verification**:
    - Status: Successfully compiled `Telegram.exe` (Release mode).
    - Output: `tdesktop/out/Release/Telegram.exe` (approx. 197MB).

## 3. Deployment & Usage

### Frontend
- **Path**: `platform/frontend/yupi-antd-frontend-init-master`
- **Start Command**: `npm run start`
- **Address**: `http://localhost:8000`
- **Note**: Requires Node.js 18+ and pnpm/npm.

### Backend
- **Path**: `platform/backend/springboot-init-master`
- **Start Command**: Run `MainApplication.java` in IDEA or `mvn spring-boot:run`.
- **Address**: `http://localhost:8101`
- **Database**: H2 In-Memory Database (`jdbc:h2:mem:testdb`).
    - **Note**: **Data is lost on backend restart**.
- **Account**:
    - No default admin account is pre-seeded.
    - Please **Register** a new account via the Frontend (`/user/register`).
    - Default password logic in code (for created users via API): `12345678`.

### Telegram Client (C2)
- **Path**: `tdesktop`
- **Build**: Use `tdesktop/compile_client.bat` (Visual Studio 2022 required).
- **Functionality**:
    - Sends heartbeat to `http://localhost:8101/api/heartbeat`.
    - Fetches tasks from `http://localhost:8101/api/c2/tasks/pending`.
    - Uploads results (CMD output, screenshots) to `http://localhost:8101/api/c2/tasks/result`.

## 4. Next Steps
1.  Unzip and analyze the `platform` backend code.
2.  Design the SQLite schema for the client.
3.  Implement the Message Hook in `tdesktop` source code.

---
*Note: This document will be updated automatically as tasks are completed.*
