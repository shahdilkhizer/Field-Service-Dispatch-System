#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;

bool isProgramExiting = false;

struct User;
struct Customer;
struct Skill;
struct WorkOrder;
struct Appointment;
struct ServiceResource;
struct Asset;

template <typename T>
struct Node {
    T data;
    Node<T> *next;
    Node(T val) : data(val), next(NULL) {}
};

template <typename T>
class LinkedList {
    Node<T> *head;
public:
    LinkedList() : head(NULL) {}
    ~LinkedList() {
        clear();
    }
    void clear() {
        Node<T> *curr = head;
        while (curr) {
            Node<T> *temp = curr;
            curr = curr->next;
            delete temp;
        }
        head = NULL;
    }
    void add(T val) {
        Node<T> *newNode = new Node<T>(val);
        if (!head) head = newNode;
        else {
            Node<T> *temp = head;
            while (temp->next) temp = temp->next;
            temp->next = newNode;
        }
    }
    bool isEmpty() const { return head == NULL; }
    Node<T> *getHead() const { return head; }
    T *searchById(int id) {
        Node<T> *curr = head;
        while (curr) {
            if (curr->data.recordId == id) return &(curr->data);
            curr = curr->next;
        }
        return NULL;
    }
    void deleteById(int id) {
        if (!head) return;
        if (head->data.recordId == id) {
            Node<T> *temp = head;
            head = head->next;
            delete temp;
            return;
        }
        Node<T> *curr = head;
        while (curr->next && curr->next->data.recordId != id) curr = curr->next;
        if (curr->next) {
            Node<T> *temp = curr->next;
            curr->next = curr->next->next;
            delete temp;
        }
    }
};

struct User {
    static int count;
    int recordId;
    string name, email, password, role;
    bool isActive;
    User() : User("Unknown", "", "", "") {}
    User(string n, string e, string p, string r) {
        recordId = count++;
        name = n; email = e; password = p; role = r; isActive = true;
    }
    void display() {
        cout << "\n===== USER =====\nID: " << recordId << "\nName: " << name << "\nEmail: " << email << "\nRole: " << role << "\nActive: " << (isActive ? "Yes" : "No") << endl;
    }
};
int User::count = 1;

struct Customer {
    static int count;
    int recordId;
    string companyName, fullName, email, phone;
    Customer() : Customer("Unknown", "", "", "") {}
    Customer(string cn, string fn, string em, string ph) {
        recordId = count++;
        companyName = cn; fullName = fn; email = em; phone = ph;
    }
    ~Customer();
    void display() {
        cout << "\n===== CUSTOMER =====\nID: " << recordId << "\nCompany: " << companyName << "\nName: " << fullName << "\nEmail: " << email << "\nPhone: " << phone << endl;
    }
    void getRelatedWO(LinkedList<WorkOrder> &resultWOList);
    void deleteRelatedWO();
};
int Customer::count = 1;

struct Skill {
    static int count;
    int recordId;
    string skillName;
    int skillLevel;
    Skill() : Skill("Unknown", -1) {}
    Skill(string sn, int lvl) {
        recordId = count++; skillName = sn; skillLevel = lvl;
    }
    ~Skill();
    void display() {
        cout << "\n===== SKILL =====\nSkill Name: " << skillName << "\nLevel: " << skillLevel << endl;
    }
    void getRelatedWO(LinkedList<WorkOrder> &resultWOList);
    void removeSkillsFromRelatedWO();
    void getServiceResource(LinkedList<ServiceResource> &resultServiceResourceList);
    void removeSkillsFromRelatedServiceResource();
};
int Skill::count = 1;

struct WorkOrder {
    static int count;
    int recordId;
    vector<int> requiredSkillSet;
    int customerId, assetId;
    string priority, status;
    WorkOrder() : WorkOrder("Low", "Unknown", -1, -1) {}
    WorkOrder(string prio, string stat, int custId, int astId) {
        recordId = count++; priority = prio; status = stat; customerId = custId; assetId = astId;
    }
    ~WorkOrder();
    void display() {
        cout << "\n===== WORK ORDER =====\nID: " << recordId << "\nCustomer ID: " << customerId << "\nAsset ID: " << assetId << "\nPriority: " << priority << "\nStatus: " << status << endl;
    }
    Node<Appointment> *getRelatedAppointment();
};
int WorkOrder::count = 1;

struct Appointment {
    static int count;
    int recordId;
    int workOrderId, technicianId, slotIndex;
    Appointment() : Appointment(-1, -1, -1) {}
    Appointment(int woId, int techId, int slot) {
        recordId = count++; workOrderId = woId; technicianId = techId; slotIndex = slot;
    }
    void display() {
        cout << "\n===== APPOINTMENT =====\nID: " << recordId << "\nWork Order ID: " << workOrderId << "\nTechnician ID: " << technicianId << "\nSlot: " << slotIndex + 1 << endl;
    }
};
int Appointment::count = 1;

enum SlotStatus { AVAILABLE, BOOKED, BLOCKED };
const int operatingMinutes = 480;
const int appointmentDurationMinutes = 30;

struct ServiceResource {
    static int count;
    int recordId;
    int userId;
    vector<int> skillSet;
    vector<SlotStatus> slots;
    ServiceResource() : ServiceResource(-1) {}
    ServiceResource(int uid) {
        recordId = count++;
        userId = uid;
        int totalSlots = operatingMinutes / appointmentDurationMinutes;
        slots.resize(totalSlots, AVAILABLE);
    }
    ~ServiceResource() {
        if (isProgramExiting) return;
        markedWOStatusPending();
    }
    int findFirstAvailableSlot() {
        for (int i = 0; i < (int)slots.size(); ++i)
            if (slots[i] == AVAILABLE) return i;
        return -1;
    }
    void display() {
        cout << "\n===== SERVICE RESOURCE =====\nID: " << recordId << "\nUser ID: " << userId << "\nSlots: ";
        for (int i = 0; i < (int)slots.size(); ++i) {
            if (slots[i] == AVAILABLE) cout << "A";
            else if (slots[i] == BOOKED) cout << "B";
            else cout << "X";
            if (i < (int)slots.size() - 1) cout << ",";
        }
        cout << endl;
    }
    int getAvailableSlotsCount() {
        int c = 0;
        for (SlotStatus s : slots) if (s == AVAILABLE) c++;
        return c;
    }
    int *getAvailableSlots() {
        int *avail = new int[getAvailableSlotsCount()];
        int ind = 0;
        for (int i = 0; i < (int)slots.size(); i++)
            if (slots[i] == AVAILABLE) avail[ind++] = i + 1;
        return avail;
    }
    void markedWOStatusPending();
    void getRelatedAppointments(LinkedList<Appointment> &resultAppointmentList);
    void blockTechnicianAvailableSlot();
    bool isResourceUserActive();
};
int ServiceResource::count = 1;

struct Asset {
    static int count;
    int recordId;
    string productName;
    Asset() : Asset("Unknown") {}
    Asset(string pn) { recordId = count++; productName = pn; }
    ~Asset() {}
    void display() {
        cout << "\n===== ASSET =====\nID: " << recordId << "\nProduct: " << productName << endl;
    }
    void getRelatedWO(LinkedList<WorkOrder> &resultWOList);
};
int Asset::count = 1;

LinkedList<User> users;
LinkedList<Customer> customers;
LinkedList<Skill> SkillSet;
LinkedList<WorkOrder> workOrders;
LinkedList<Asset> assets;
LinkedList<ServiceResource> technicians;
LinkedList<Appointment> appointments;

void Customer::getRelatedWO(LinkedList<WorkOrder> &resultWOList) {
    Node<WorkOrder> *woList = workOrders.getHead();
    while (woList) {
        if (woList->data.customerId == recordId) resultWOList.add(woList->data);
        woList = woList->next;
    }
}

void Customer::deleteRelatedWO() {
    LinkedList<WorkOrder> delWO;
    getRelatedWO(delWO);
    if (!delWO.isEmpty()) {
        Node<WorkOrder> *delWOList = delWO.getHead();
        while (delWOList) {
            Node<WorkOrder> *nextDelWO = delWOList->next;
            workOrders.deleteById(delWOList->data.recordId);
            delWOList = nextDelWO;
        }
    }
}
Customer::~Customer() { if (isProgramExiting) return; deleteRelatedWO(); }

void Skill::getRelatedWO(LinkedList<WorkOrder> &resultWOList) {
    Node<WorkOrder> *woList = workOrders.getHead();
    while (woList) {
        for (int id : woList->data.requiredSkillSet) {
            if (id == recordId) { resultWOList.add(woList->data); break; }
        }
        woList = woList->next;
    }
}

void Skill::removeSkillsFromRelatedWO() {
    LinkedList<WorkOrder> relWO;
    getRelatedWO(relWO);
    Node<WorkOrder> *woList = relWO.getHead();
    while (woList) {
        WorkOrder *wo = workOrders.searchById(woList->data.recordId);
        if (wo) {
            vector<int> &sk = wo->requiredSkillSet;
            sk.erase(remove(sk.begin(), sk.end(), recordId), sk.end());
        }
        woList = woList->next;
    }
}

void Skill::getServiceResource(LinkedList<ServiceResource> &resultServiceResourceList) {
    Node<ServiceResource> *techList = technicians.getHead();
    while (techList) {
        for (int id : techList->data.skillSet) {
            if (id == recordId) { resultServiceResourceList.add(techList->data); break; }
        }
        techList = techList->next;
    }
}

void Skill::removeSkillsFromRelatedServiceResource() {
    LinkedList<ServiceResource> relTech;
    getServiceResource(relTech);
    Node<ServiceResource> *techList = relTech.getHead();
    while (techList) {
        ServiceResource *tech = technicians.searchById(techList->data.recordId);
        if (tech) {
            vector<int> &sk = tech->skillSet;
            sk.erase(remove(sk.begin(), sk.end(), recordId), sk.end());
        }
        techList = techList->next;
    }
}
Skill::~Skill() {
    if (isProgramExiting) return;
    removeSkillsFromRelatedWO();
    removeSkillsFromRelatedServiceResource();
}

WorkOrder::~WorkOrder() {
    if (isProgramExiting) return;
    if (status != "Pending") {
        Node<Appointment> *delAppointment = getRelatedAppointment();
        if (delAppointment != NULL) {
            ServiceResource *tech = technicians.searchById(delAppointment->data.technicianId);
            if (tech && delAppointment->data.slotIndex >= 0 && delAppointment->data.slotIndex < (int)tech->slots.size()) {
                tech->slots[delAppointment->data.slotIndex] = AVAILABLE;
            }
            appointments.deleteById(delAppointment->data.recordId);
        }
    }
}

Node<Appointment> *WorkOrder::getRelatedAppointment() {
    Node<Appointment> *appointment = appointments.getHead();
    while (appointment) {
        if (appointment->data.workOrderId == recordId) return appointment;
        appointment = appointment->next;
    }
    return NULL;
}

void Asset::getRelatedWO(LinkedList<WorkOrder> &resultWOList) {
    Node<WorkOrder> *woList = workOrders.getHead();
    while (woList) {
        if (woList->data.assetId == recordId) resultWOList.add(woList->data);
        woList = woList->next;
    }
}

void ServiceResource::getRelatedAppointments(LinkedList<Appointment> &resultAppointmentList) {
    Node<Appointment> *appList = appointments.getHead();
    while (appList) {
        if (appList->data.technicianId == recordId) resultAppointmentList.add(appList->data);
        appList = appList->next;
    }
}

void ServiceResource::markedWOStatusPending() {
    LinkedList<Appointment> relApp;
    getRelatedAppointments(relApp);
    Node<Appointment> *appList = relApp.getHead();
    vector<int> toDelete;
    while (appList) {
        if (appList->data.technicianId == recordId) {
            WorkOrder *wo = workOrders.searchById(appList->data.workOrderId);
            if (wo) wo->status = "Pending";
            if (appList->data.slotIndex >= 0 && appList->data.slotIndex < (int)slots.size())
                slots[appList->data.slotIndex] = AVAILABLE;
            toDelete.push_back(appList->data.recordId);
        }
        appList = appList->next;
    }
    for (int id : toDelete) appointments.deleteById(id);
}

void ServiceResource::blockTechnicianAvailableSlot() {
    User *u = users.searchById(userId);
    string techName = (u != NULL) ? u->name : ("Tech ID " + to_string(recordId));
    if (!isResourceUserActive()) {
        cout << "Technician " << techName << " is inActive, Kindly activate the user related to this Service Resource to blocked the slots !!\n";
        return;
    }
    int availableSlotsCount = getAvailableSlotsCount();
    if (availableSlotsCount == 0) {
        cout << "Technician " << techName << " has no available slots to mark as block !!\n";
        return;
    }
    int *availableSlots = getAvailableSlots();
    for (int i = 0; i < availableSlotsCount; i++) {
        cout << availableSlots[i];
        if (i != availableSlotsCount - 1) cout << ", ";
    }
    cout << endl;
    int slotNum;
    cout << " Enter available slot number : ";
    if (!(cin >> slotNum)) {
        delete[] availableSlots; cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input! Returning to main menu.\n"; return;
    }
    cin.ignore();
    bool slotFound = false;
    for (int i = 0; i < availableSlotsCount; i++)
        if (slotNum == availableSlots[i]) slotFound = true;
    delete[] availableSlots;
    if (!slotFound) {
        cout << "Your selected slot is not present in the available slot list. Returning to main menu.\n";
        return;
    }
    slots[slotNum - 1] = BLOCKED;
    cout << "Slot has been BLOCKED for this Technician. Happy Vacation !!!!!\n";
}

bool ServiceResource::isResourceUserActive() {
    User *u = users.searchById(userId);
    return (u != NULL && u->isActive);
}

string getDataDir() {
    ifstream inParent("../data/users.csv");
    if (inParent.is_open()) { inParent.close(); return "../data/"; }
    ifstream inRoot("data/users.csv");
    if (inRoot.is_open()) { inRoot.close(); return "data/"; }
    ifstream checkSrcFile("Main.cpp");
    if (checkSrcFile.is_open()) { checkSrcFile.close(); return "../data/"; }
    return "data/";
}

string getUserFile() { return getDataDir() + "users.csv"; }
string getCustomerFile() { return getDataDir() + "customers.csv"; }
string getSkillFile() { return getDataDir() + "skills.csv"; }
string getWorkOrderFile() { return getDataDir() + "workorders.csv"; }
string getAssetFile() { return getDataDir() + "assets.csv"; }
string getTechnicianFile() { return getDataDir() + "technicians.csv"; }
string getAppointmentFile() { return getDataDir() + "appointments.csv"; }

void createDataDir() {
    string d = getDataDir();
#ifdef _WIN32
    string cmd = "mkdir " + d + " 2>nul";
    system(cmd.c_str());
#else
    string cmd = "mkdir -p " + d;
    system(cmd.c_str());
#endif
}

void updateCounters() {
    int maxId = 0;
    Node<User> *u = users.getHead();
    while (u) { maxId = max(maxId, u->data.recordId); u = u->next; }
    User::count = maxId + 1;
    maxId = 0;
    Node<Customer> *c = customers.getHead();
    while (c) { maxId = max(maxId, c->data.recordId); c = c->next; }
    Customer::count = maxId + 1;
    maxId = 0;
    Node<Skill> *s = SkillSet.getHead();
    while (s) { maxId = max(maxId, s->data.recordId); s = s->next; }
    Skill::count = maxId + 1;
    maxId = 0;
    Node<WorkOrder> *w = workOrders.getHead();
    while (w) { maxId = max(maxId, w->data.recordId); w = w->next; }
    WorkOrder::count = maxId + 1;
    maxId = 0;
    Node<Asset> *a = assets.getHead();
    while (a) { maxId = max(maxId, a->data.recordId); a = a->next; }
    Asset::count = maxId + 1;
    maxId = 0;
    Node<ServiceResource> *t = technicians.getHead();
    while (t) { maxId = max(maxId, t->data.recordId); t = t->next; }
    ServiceResource::count = maxId + 1;
    maxId = 0;
    Node<Appointment> *ap = appointments.getHead();
    while (ap) { maxId = max(maxId, ap->data.recordId); ap = ap->next; }
    Appointment::count = maxId + 1;
}

void saveUsers() {
    ofstream out(getUserFile());
    Node<User> *curr = users.getHead();
    while (curr) {
        User &u = curr->data;
        out << u.recordId << "," << u.name << "," << u.email << "," << u.password << "," << u.role << "," << (u.isActive ? "1" : "0") << "\n";
        curr = curr->next;
    }
}

void saveCustomers() {
    ofstream out(getCustomerFile());
    Node<Customer> *curr = customers.getHead();
    while (curr) {
        Customer &c = curr->data;
        out << c.recordId << "," << c.companyName << "," << c.fullName << "," << c.email << "," << c.phone << "\n";
        curr = curr->next;
    }
}

void saveSkills() {
    ofstream out(getSkillFile());
    Node<Skill> *curr = SkillSet.getHead();
    while (curr) {
        Skill &s = curr->data;
        out << s.recordId << "," << s.skillName << "," << s.skillLevel << "\n";
        curr = curr->next;
    }
}

void saveWorkOrders() {
    ofstream out(getWorkOrderFile());
    Node<WorkOrder> *curr = workOrders.getHead();
    while (curr) {
        WorkOrder &w = curr->data;
        out << w.recordId << "," << w.customerId << "," << w.assetId << "," << w.priority << "," << w.status << ",";
        for (int i = 0; i < (int)w.requiredSkillSet.size(); ++i) {
            if (i > 0) out << ";";
            out << w.requiredSkillSet[i];
        }
        out << "\n";
        curr = curr->next;
    }
}

void saveAssets() {
    ofstream out(getAssetFile());
    Node<Asset> *curr = assets.getHead();
    while (curr) {
        Asset &a = curr->data;
        out << a.recordId << "," << a.productName << "\n";
        curr = curr->next;
    }
}

void saveTechnicians() {
    ofstream out(getTechnicianFile());
    Node<ServiceResource> *curr = technicians.getHead();
    while (curr) {
        ServiceResource &t = curr->data;
        out << t.recordId << "," << t.userId << ",";
        for (int i = 0; i < (int)t.skillSet.size(); ++i) {
            if (i > 0) out << ";";
            out << t.skillSet[i];
        }
        out << ",";
        for (int i = 0; i < (int)t.slots.size(); ++i) {
            if (i > 0) out << ";";
            out << (int)t.slots[i];
        }
        out << "\n";
        curr = curr->next;
    }
}

void saveAppointments() {
    ofstream out(getAppointmentFile());
    Node<Appointment> *curr = appointments.getHead();
    while (curr) {
        Appointment &a = curr->data;
        out << a.recordId << "," << a.workOrderId << "," << a.technicianId << "," << a.slotIndex << "\n";
        curr = curr->next;
    }
}

void saveAll() {
    saveUsers(); saveCustomers(); saveSkills(); saveWorkOrders();
    saveAssets(); saveTechnicians(); saveAppointments();
    cout << "All data saved to " << getDataDir() << "\n";
}

void loadUsers() {
    ifstream in(getUserFile());
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, name, email, pass, role, activeStr;
        getline(ss, id, ','); getline(ss, name, ','); getline(ss, email, ',');
        getline(ss, pass, ','); getline(ss, role, ','); getline(ss, activeStr, ',');
        if (id.empty()) continue;
        User u(name, email, pass, role);
        u.recordId = stoi(id);
        if (!activeStr.empty()) u.isActive = (activeStr == "1");
        users.add(u);
    }
}

void loadCustomers() {
    ifstream in(getCustomerFile());
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, cn, fn, em, ph;
        getline(ss, id, ','); getline(ss, cn, ','); getline(ss, fn, ',');
        getline(ss, em, ','); getline(ss, ph, ',');
        if (id.empty()) continue;
        Customer c(cn, fn, em, ph);
        c.recordId = stoi(id);
        customers.add(c);
    }
}

void loadSkills() {
    ifstream in(getSkillFile());
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, name, lvl;
        getline(ss, id, ','); getline(ss, name, ','); getline(ss, lvl, ',');
        if (id.empty() || lvl.empty()) continue;
        Skill s(name, stoi(lvl));
        s.recordId = stoi(id);
        SkillSet.add(s);
    }
}

void loadWorkOrders() {
    ifstream in(getWorkOrderFile());
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, cid, aid, prio, stat, skillsStr;
        getline(ss, id, ','); getline(ss, cid, ','); getline(ss, aid, ',');
        getline(ss, prio, ','); getline(ss, stat, ','); getline(ss, skillsStr, ',');
        if (id.empty() || cid.empty() || aid.empty()) continue;
        WorkOrder w(prio, stat, stoi(cid), stoi(aid));
        w.recordId = stoi(id);
        stringstream skillStream(skillsStr);
        string skillId;
        while (getline(skillStream, skillId, ';'))
            if (!skillId.empty()) w.requiredSkillSet.push_back(stoi(skillId));
        workOrders.add(w);
    }
}

void loadAssets() {
    ifstream in(getAssetFile());
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, pn;
        getline(ss, id, ','); getline(ss, pn, ',');
        if (id.empty()) continue;
        Asset a(pn);
        a.recordId = stoi(id);
        assets.add(a);
    }
}

void loadTechnicians() {
    ifstream in(getTechnicianFile());
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, uid, skillsStr, slotsStr;
        getline(ss, id, ','); getline(ss, uid, ','); getline(ss, skillsStr, ','); getline(ss, slotsStr, ',');
        if (id.empty() || uid.empty()) continue;
        ServiceResource t(stoi(uid));
        t.recordId = stoi(id);
        stringstream skillStream(skillsStr);
        string skillId;
        while (getline(skillStream, skillId, ';'))
            if (!skillId.empty()) t.skillSet.push_back(stoi(skillId));
        stringstream slotStream(slotsStr);
        string slotVal;
        int idx = 0;
        while (getline(slotStream, slotVal, ';'))
            if (!slotVal.empty() && idx < (int)t.slots.size()) t.slots[idx++] = (SlotStatus)stoi(slotVal);
        technicians.add(t);
    }
}

void loadAppointments() {
    ifstream in(getAppointmentFile());
    if (!in.is_open()) return;
    string line;
    while (getline(in, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string id, woId, techId, slot;
        getline(ss, id, ','); getline(ss, woId, ','); getline(ss, techId, ','); getline(ss, slot, ',');
        if (id.empty() || woId.empty() || techId.empty() || slot.empty()) continue;
        Appointment a(stoi(woId), stoi(techId), stoi(slot));
        a.recordId = stoi(id);
        appointments.add(a);
    }
}

void loadAll() {
    loadUsers(); loadCustomers(); loadSkills(); loadWorkOrders();
    loadAssets(); loadTechnicians(); loadAppointments();
    updateCounters();
    cout << "All data loaded from " << getDataDir() << "\n";
}

int woPriorityHeap[500];
int woIdHeap[500];
int woHeapSize = 0;
int woHeapCapacity = 500;

int getPriorityValue(string priority) {
    if (priority == "High") return 3;
    if (priority == "Medium") return 2;
    if (priority == "Low") return 1;
    return 0;
}

void insertWorkOrder(int priorityValue, int workOrderId) {
    if (woHeapSize >= woHeapCapacity) {
        cout << "Heap capacity reached (" << woHeapCapacity << "). Cannot insert more work orders.\n";
        return;
    }
    int i = woHeapSize;
    woPriorityHeap[i] = priorityValue;
    woIdHeap[i] = workOrderId;
    woHeapSize++;

    while (i != 0 && woPriorityHeap[(i - 1) / 2] < woPriorityHeap[i]) {
        swap(woPriorityHeap[i], woPriorityHeap[(i - 1) / 2]);
        swap(woIdHeap[i], woIdHeap[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

void maxHeapify(int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < woHeapSize && woPriorityHeap[left] > woPriorityHeap[largest]) largest = left;
    if (right < woHeapSize && woPriorityHeap[right] > woPriorityHeap[largest]) largest = right;

    if (largest != i) {
        swap(woPriorityHeap[i], woPriorityHeap[largest]);
        swap(woIdHeap[i], woIdHeap[largest]);
        maxHeapify(largest);
    }
}

int extractMaxWorkOrder(int &outPriorityValue) {
    if (woHeapSize <= 0) return -1;
    if (woHeapSize == 1) {
        woHeapSize--;
        outPriorityValue = woPriorityHeap[0];
        return woIdHeap[0];
    }
    int rootId = woIdHeap[0];
    outPriorityValue = woPriorityHeap[0];
    woPriorityHeap[0] = woPriorityHeap[woHeapSize - 1];
    woIdHeap[0] = woIdHeap[woHeapSize - 1];
    woHeapSize--;
    maxHeapify(0);
    return rootId;
}

bool isHeapEmpty() { return woHeapSize == 0; }

void loadPendingWorkOrders() {
    woHeapSize = 0;
    Node<WorkOrder> *curr = workOrders.getHead();
    int count = 0;
    while (curr) {
        if (curr->data.status == "Pending") {
            int pVal = getPriorityValue(curr->data.priority);
            insertWorkOrder(pVal, curr->data.recordId);
            count++;
        }
        curr = curr->next;
    }
    cout << "Loaded " << count << " pending work orders into heap.\n";
}

bool hasRequiredSkills(ServiceResource &tech, WorkOrder &wo) {
    for (int req : wo.requiredSkillSet) {
        bool found = false;
        for (int tskill : tech.skillSet) {
            if (tskill == req) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

void dispatchAll() {
    loadPendingWorkOrders();
    if (isHeapEmpty()) {
        cout << "No pending work orders to dispatch.\n";
        return;
    }
    int remainingWOCount = 0;
    while (!isHeapEmpty()) {
        int pVal;
        int woId = extractMaxWorkOrder(pVal);
        WorkOrder *wo = workOrders.searchById(woId);
        if (!wo) continue;
        if (wo->assetId == -1 || !assets.searchById(wo->assetId)) {
            cout << "Could not assign - invalid or missing Asset ID for Work Order Id " << wo->recordId << ". Kindly assign a valid Asset to this Work Order.\n";
            remainingWOCount++; continue;
        }
        if (wo->requiredSkillSet.empty()) {
            cout << "Could not assign - no skillSet found for Work Order Id " << wo->recordId << ". Kindly add Required Skills for this Work Order.\n";
            remainingWOCount++; continue;
        }
        cout << "\n--- Dispatching Work Order #" << woId << " (Priority: " << wo->priority << ") ---\n";
        bool assigned = false;

        Node<ServiceResource> *techNode = technicians.getHead();
        int maxAvailableSlotTechinician = 0;
        ServiceResource *maxAvailableSlotTech = NULL;

        while (techNode) {
            ServiceResource &tech = techNode->data;
            if (tech.isResourceUserActive() && hasRequiredSkills(tech, *wo)) {
                if (tech.getAvailableSlotsCount() > maxAvailableSlotTechinician) {
                    maxAvailableSlotTechinician = tech.getAvailableSlotsCount();
                    maxAvailableSlotTech = &tech;
                }
            }
            techNode = techNode->next;
        }
        if (maxAvailableSlotTechinician > 0) {
            int slot = maxAvailableSlotTech->findFirstAvailableSlot();
            if (slot != -1) {
                maxAvailableSlotTech->slots[slot] = BOOKED;
                wo->status = "Assigned";
                Appointment apt(wo->recordId, maxAvailableSlotTech->recordId, slot);
                appointments.add(apt);
                cout << "Appointment #" << apt.recordId << " created and Assigned to Technician ID " << maxAvailableSlotTech->recordId << " at Slot " << (slot + 1) << "\n";
                assigned = true;
            }
        }
        else {
            remainingWOCount++;
        }
        if (!assigned) {
            cout << "   Could not assign - no matching technician with free slot for Work Order Id " << wo->recordId << ".\n";
            remainingWOCount++;
        }
    }
    saveAll();
    if (remainingWOCount > 0)
        cout << remainingWOCount << " work orders still pending after dispatch. Complete some appointments and re-run dispatch.\n";
    cout << "\nDispatch completed. All data saved.\n";
}

void addSampleData() {
    users.add(User("Admin User", "admin@fieldservice.com", "admin123", "admin"));
    users.add(User("John Smith", "john@tech.com", "pass123", "technician"));
    users.add(User("Jane Doe", "jane@tech.com", "pass456", "technician"));
    users.add(User("Michael Chang", "michael@tech.com", "pass789", "technician"));
    users.add(User("Dispatcher One", "dispatch@fieldservice.com", "disp123", "dispatcher"));

    customers.add(Customer("ABC Corp", "Alice Johnson", "alice@abccorp.com", "555-0101"));
    customers.add(Customer("XYZ Ltd", "Bob Williams", "bob@xyz.com", "555-0102"));
    customers.add(Customer("Smith & Sons", "Carol Smith", "carol@smith.com", "555-0103"));
    customers.add(Customer("Metro Hospital", "David Miller", "david@metrohosp.com", "555-0104"));
    customers.add(Customer("", "Emma Watson", "emma@watson.com", "555-0105"));

    assets.add(Asset("Boiler Model X"));
    assets.add(Asset("Air Conditioner Pro"));
    assets.add(Asset("Electrical Panel 3-Phase"));
    assets.add(Asset("Plumbing Pipe System"));
    assets.add(Asset("Commercial Furnace"));
    assets.add(Asset("Solar Inverter 5000W"));
    assets.add(Asset("Server Rack Air Cooler"));
    assets.add(Asset("Water Filtration Plant"));
    assets.add(Asset("Backup Diesel Generator"));
    assets.add(Asset("Elevator Control Module"));

    SkillSet.add(Skill("Plumbing", 5));
    SkillSet.add(Skill("Electrical", 4));
    SkillSet.add(Skill("HVAC", 5));
    SkillSet.add(Skill("Carpentry", 3));
    SkillSet.add(Skill("General Maintenance", 2));
    SkillSet.add(Skill("Solar Power Systems", 4));
    SkillSet.add(Skill("Network Cabling", 3));

    ServiceResource tech1(2);
    tech1.skillSet.push_back(1); tech1.skillSet.push_back(2); tech1.skillSet.push_back(5);
    tech1.slots[0] = BOOKED; tech1.slots[1] = BOOKED;
    technicians.add(tech1);

    ServiceResource tech2(3);
    tech2.skillSet.push_back(1); tech2.skillSet.push_back(3); tech2.skillSet.push_back(5);
    tech2.slots[0] = BOOKED; tech2.slots[1] = BOOKED;
    technicians.add(tech2);

    ServiceResource tech3(4);
    tech3.skillSet.push_back(2); tech3.skillSet.push_back(6); tech3.skillSet.push_back(7);
    tech3.slots[0] = BOOKED;
    technicians.add(tech3);

    // 5 Assigned Work Orders
    WorkOrder wo1("High", "Assigned", 1, 1); wo1.requiredSkillSet.push_back(1); workOrders.add(wo1);
    WorkOrder wo2("Medium", "Assigned", 1, 2); wo2.requiredSkillSet.push_back(2); workOrders.add(wo2);
    WorkOrder wo3("High", "Assigned", 2, 3); wo3.requiredSkillSet.push_back(2); wo3.requiredSkillSet.push_back(7); workOrders.add(wo3);
    WorkOrder wo4("Medium", "Assigned", 3, 5); wo4.requiredSkillSet.push_back(3); workOrders.add(wo4);
    WorkOrder wo5("High", "Assigned", 4, 7); wo5.requiredSkillSet.push_back(3); wo5.requiredSkillSet.push_back(5); workOrders.add(wo5);

    // 10 Pending Work Orders (Mix of High, Medium, Low priorities)
    WorkOrder wo6("High", "Pending", 4, 6); wo6.requiredSkillSet.push_back(2); wo6.requiredSkillSet.push_back(6); workOrders.add(wo6);
    WorkOrder wo7("High", "Pending", 2, 3); wo7.requiredSkillSet.push_back(2); wo7.requiredSkillSet.push_back(7); workOrders.add(wo7);
    WorkOrder wo8("High", "Pending", 1, 9); wo8.requiredSkillSet.push_back(2); workOrders.add(wo8);
    WorkOrder wo9("Medium", "Pending", 3, 5); wo9.requiredSkillSet.push_back(3); workOrders.add(wo9);
    WorkOrder wo10("Medium", "Pending", 5, 2); wo10.requiredSkillSet.push_back(3); wo10.requiredSkillSet.push_back(5); workOrders.add(wo10);
    WorkOrder wo11("Medium", "Pending", 2, 8); wo11.requiredSkillSet.push_back(1); wo11.requiredSkillSet.push_back(5); workOrders.add(wo11);
    WorkOrder wo12("Medium", "Pending", 4, 10); wo12.requiredSkillSet.push_back(2); wo12.requiredSkillSet.push_back(5); workOrders.add(wo12);
    WorkOrder wo13("Low", "Pending", 2, 4); wo13.requiredSkillSet.push_back(1); wo13.requiredSkillSet.push_back(5); workOrders.add(wo13);
    WorkOrder wo14("Low", "Pending", 3, 8); wo14.requiredSkillSet.push_back(1); workOrders.add(wo14);
    WorkOrder wo15("Low", "Pending", 5, 1); wo15.requiredSkillSet.push_back(1); wo15.requiredSkillSet.push_back(5); workOrders.add(wo15);

    // 3 Completed Work Orders
    WorkOrder wo16("Low", "Completed", 1, 1); wo16.requiredSkillSet.push_back(1); workOrders.add(wo16);
    WorkOrder wo17("Low", "Completed", 3, 4); wo17.requiredSkillSet.push_back(1); wo17.requiredSkillSet.push_back(5); workOrders.add(wo17);
    WorkOrder wo18("Low", "Completed", 4, 9); wo18.requiredSkillSet.push_back(2); workOrders.add(wo18);

    // 5 Appointments matching Assigned Work Orders
    appointments.add(Appointment(1, 1, 0));
    appointments.add(Appointment(2, 1, 1));
    appointments.add(Appointment(3, 3, 0));
    appointments.add(Appointment(4, 2, 0));
    appointments.add(Appointment(5, 2, 1));

    updateCounters();
    saveAll();
    cout << "Sample data added successfully.\n";
}

void displayUsersTable() {
    cout << "\n============ USERS ============\n";
    if (users.isEmpty()) { cout << "No users found.\n"; return; }
    cout << "ID\tName\t\tRole\n--------------------------------\n";
    Node<User> *curr = users.getHead();
    while (curr) {
        cout << curr->data.recordId << "\t" << curr->data.name << "\t\t" << curr->data.role << endl;
        curr = curr->next;
    }
}

void displayCustomersTable() {
    cout << "\n================= CUSTOMERS =================\n";
    if (customers.isEmpty()) { cout << "No customers found.\n"; return; }
    cout << "ID\tCompany Name\t\tContact Person\n------------------------------------------------\n";
    Node<Customer> *curr = customers.getHead();
    while (curr) {
        cout << curr->data.recordId << "\t" << curr->data.companyName << "\t\t" << curr->data.fullName << endl;
        curr = curr->next;
    }
}

void displaySkillsTable() {
    cout << "\n======== SKILLS ========\n";
    if (SkillSet.isEmpty()) { cout << "No skills found.\n"; return; }
    cout << "ID\tLevel\tSkill Name\n----------------------------\n";
    Node<Skill> *curr = SkillSet.getHead();
    while (curr) {
        cout << curr->data.recordId << "\t" << curr->data.skillLevel << "\t\t" << curr->data.skillName << endl;
        curr = curr->next;
    }
}

void displayAssetsTable() {
    cout << "\n=============== ASSETS ===============\n";
    if (assets.isEmpty()) { cout << "No assets found.\n"; return; }
    cout << "ID\tProduct Name\n----------------------------------------\n";
    Node<Asset> *curr = assets.getHead();
    while (curr) {
        cout << curr->data.recordId << "\t" << curr->data.productName << endl;
        curr = curr->next;
    }
}

void displayTechniciansTable() {
    cout << "\n============================== TECHNICIANS ==============================\n";
    if (technicians.isEmpty()) { cout << "No technicians found.\n"; return; }
    cout << "ID\tUser (ID & Name)\t\tSkills\n-------------------------------------------------------------------------\n";
    Node<ServiceResource> *curr = technicians.getHead();
    while (curr) {
        User *u = users.searchById(curr->data.userId);
        string uname = u ? u->name : "Unknown";
        cout << curr->data.recordId << "\t" << curr->data.userId << " (" << uname << ")\t\t";
        for (int i = 0; i < (int)curr->data.skillSet.size(); i++) {
            if (i > 0) cout << ", ";
            Skill *s = SkillSet.searchById(curr->data.skillSet[i]);
            if (s) cout << s->skillName;
        }
        cout << endl;
        curr = curr->next;
    }
}

string getCustomerDisplayName(Customer *c) {
    if (!c) return "Unknown";
    if (!c->companyName.empty()) return c->companyName;
    if (!c->fullName.empty()) return c->fullName;
    return "Unknown";
}

void displayWorkOrdersTable() {
    cout << "\n========================================= WORK ORDERS =========================================\n";
    if (workOrders.isEmpty()) { cout << "No work orders found.\n"; return; }
    cout << "ID\tCustomer (ID & Name)\t\tAsset (ID & Name)\t\tPriority\tStatus\t\tRequired Skills\n";
    cout << "---------------------------------------------------------------------------------------------------------------\n";
    Node<WorkOrder> *curr = workOrders.getHead();
    while (curr) {
        WorkOrder &wo = curr->data;
        Customer *c = customers.searchById(wo.customerId);
        Asset *a = assets.searchById(wo.assetId);
        string cname = getCustomerDisplayName(c);
        string aname = a ? a->productName : "None";
        cout << wo.recordId << "\t" << wo.customerId << " (" << cname << ")\t\t" << wo.assetId << " (" << aname << ")\t\t" << wo.priority << "\t\t" << wo.status << "\t\t";
        if (wo.requiredSkillSet.empty()) {
            cout << "None";
        } else {
            for (int i = 0; i < (int)wo.requiredSkillSet.size(); i++) {
                if (i > 0) cout << ", ";
                Skill *s = SkillSet.searchById(wo.requiredSkillSet[i]);
                if (s) cout << s->skillName;
            }
        }
        cout << endl;
        curr = curr->next;
    }
}

void displayAppointmentsTable() {
    cout << "\n================================= APPOINTMENTS =================================\n";
    if (appointments.isEmpty()) { cout << "No appointments found.\n"; return; }
    cout << "ID\tWork Order ID\tTechnician (ID & Name)\t\tSlot\n";
    cout << "--------------------------------------------------------------------------------\n";
    Node<Appointment> *curr = appointments.getHead();
    while (curr) {
        Appointment &a = curr->data;
        ServiceResource *tech = technicians.searchById(a.technicianId);
        User *u = tech ? users.searchById(tech->userId) : NULL;
        string tname = u ? u->name : "Unknown Tech";
        cout << a.recordId << "\t" << a.workOrderId << "\t\t" << a.technicianId << " (" << tname << ")\t\tSlot " << (a.slotIndex + 1) << endl;
        curr = curr->next;
    }
}

void showUtilization() {
    cout << "\n===== TECHNICIAN UTILIZATION =====\n";
    Node<ServiceResource> *curr = technicians.getHead();
    while (curr) {
        User *u = users.searchById(curr->data.userId);
        string uname = u ? u->name : "Unknown";
        int total = curr->data.slots.size();
        int booked = total - curr->data.getAvailableSlotsCount();
        float percent = (booked * 100.0) / total;
        cout << "Tech ID " << curr->data.recordId << " (" << uname << "): " << booked << "/" << total << " slots booked (" << percent << "%)\n";
        curr = curr->next;
    }
}

void showPendingWorkOrders() {
    cout << "\n===== PENDING WORK ORDERS =====\n";
    Node<WorkOrder> *curr = workOrders.getHead();
    bool found = false;
    while (curr) {
        if (curr->data.status == "Pending") {
            Customer *c = customers.searchById(curr->data.customerId);
            Asset *a = assets.searchById(curr->data.assetId);
            string cname = getCustomerDisplayName(c);
            string aname = a ? a->productName : "None";
            cout << "ID: " << curr->data.recordId << " | Priority: " << curr->data.priority << " | Customer: " << curr->data.customerId << " (" << cname << ") | Asset: " << curr->data.assetId << " (" << aname << ")\n";
            found = true;
        }
        curr = curr->next;
    }
    if (!found) cout << "No pending work orders.\n";
}

void completeAppointment(int appointmentId) {
    Appointment *apt = appointments.searchById(appointmentId);
    if (!apt) {
        cout << "Appointment ID " << appointmentId << " not found! Returning to main menu.\n";
        return;
    }
    ServiceResource *tech = technicians.searchById(apt->technicianId);
    if (tech) {
        tech->slots[apt->slotIndex] = AVAILABLE;
        cout << "Appointment #" << appointmentId << " completed. Slot freed.\n";
    }
    int woId = apt->workOrderId;
    appointments.deleteById(appointmentId);
    WorkOrder *wo = workOrders.searchById(woId);
    if (wo) wo->status = "Completed";
    saveAll();
}

void completeAllAppointments() {
    Node<Appointment> *appointmentList = appointments.getHead();
    Node<Appointment> *nextAppointment = NULL;
    bool isPendingAppointments = (appointmentList != NULL);
    while (appointmentList) {
        nextAppointment = appointmentList->next;
        completeAppointment(appointmentList->data.recordId);
        appointmentList = nextAppointment;
    }
    cout << (isPendingAppointments ? "All Appointments marked completed\n" : "No Pending Appointments remains\n");
}

ServiceResource *findTechnicianById(int id) {
    ServiceResource *tech = technicians.searchById(id);
    if (tech != NULL) return tech;
    Node<ServiceResource> *curr = technicians.getHead();
    while (curr != NULL) {
        if (curr->data.userId == id) return &curr->data;
        curr = curr->next;
    }
    return NULL;
}

void createAppointment() {
    if (workOrders.isEmpty()) { cout << "No Work Order\n"; return; }
    if (technicians.isEmpty()) { cout << "No Technician found. Kindly add technician for the Work Orders\n"; return; }
    int woId, techId, slotNum;
    displayWorkOrdersTable();
    cout << "Enter Work Order Id: ";
    if (!(cin >> woId)) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input! Returning to main menu.\n"; return;
    }
    cin.ignore();
    WorkOrder *wo = workOrders.searchById(woId);
    if (wo == NULL) { cout << "No Work Order found with Id " << woId << ". Returning to main menu.\n"; return; }
    else if (wo->status != "Pending") { cout << "Work Order ID " << woId << " is not in Pending status. Returning to main menu.\n"; return; }
    displayTechniciansTable();
    cout << "Enter Technician Id: ";
    if (!(cin >> techId)) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input! Returning to main menu.\n"; return;
    }
    cin.ignore();
    ServiceResource *tech = findTechnicianById(techId);
    if (!tech) { cout << "No Technician found with Id " << techId << ". Returning to main menu.\n"; return; }
    if (!hasRequiredSkills(*tech, *wo)) {
        cout << "Technician Id " << tech->recordId << " has no appropriate skills for this Work Order. Kindly select another Technician with skills that match Required Work Order.\n";
        return;
    }
    int availableSlotsCount = tech->getAvailableSlotsCount();
    if (availableSlotsCount == 0) { cout << "This technician has no available slot. Kindly select another technician for this Work Order.\n"; return; }

    cout << "Select any slot for appointment from these available slots: ";
    int *availableSlots = tech->getAvailableSlots();
    for (int i = 0; i < availableSlotsCount; i++) {
        cout << availableSlots[i];
        if (i != availableSlotsCount - 1) cout << ", ";
    }
    cout << endl;
    cout << " Enter available slot number : ";
    if (!(cin >> slotNum)) {
        delete[] availableSlots; cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input! Returning to main menu.\n"; return;
    }
    cin.ignore();
    bool slotFound = false;
    for (int i = 0; i < availableSlotsCount; i++)
        if (slotNum == availableSlots[i]) slotFound = true;
    delete[] availableSlots;
    if (!slotFound) { cout << "Your selected slot is not present in the available slot list. Returning to main menu.\n"; return; }
    appointments.add(Appointment(woId, tech->recordId, slotNum - 1));
    wo->status = "Assigned";
    tech->slots[slotNum - 1] = BOOKED;
    saveAll();
    cout << "Appointment has been created with WO Id :" << woId << " with Technician id :" << tech->recordId << " for Slot Number " << slotNum << endl;
}

void case1() {
    string n, e, p, r;
    cout << "Name: "; getline(cin, n);
    cout << "Email: "; getline(cin, e);
    Node<User> *curr = users.getHead();
    while (curr) {
        if (curr->data.email == e) { cout << "User with email '" << e << "' already exists! User not added.\n"; return; }
        curr = curr->next;
    }
    cout << "Password: "; getline(cin, p);
    cout << "Role (1. admin 2. technician 3. dispatcher | default technician will set): ";
    int rol = -1; cin >> rol; cin.ignore();
    if (rol == 1) r = "admin";
    else if (rol == 2) r = "technician";
    else if (rol == 3) r = "dispatcher";
    else r = "technician";
    User u(n, e, p, r);
    users.add(u);
    cout << "User added successfully with ID " << u.recordId << endl;
    if (r == "technician") {
        ServiceResource tech(u.recordId);
        technicians.add(tech);
        cout << "Technician profile automatically created with ID " << tech.recordId << endl;
    }
}

void case2() {
    string cn, fn, e, ph;
    cout << "Company Name: "; getline(cin, cn);
    cout << "Contact Person Name: "; getline(cin, fn);
    cout << "Email: "; getline(cin, e);
    Node<Customer> *curr = customers.getHead();
    while (curr) {
        if (curr->data.email == e) { cout << "Customer with email '" << e << "' already exists! Customer not added.\n"; return; }
        curr = curr->next;
    }
    cout << "Phone: "; getline(cin, ph);
    Customer c(cn, fn, e, ph);
    customers.add(c);
    cout << "Customer added successfully with ID " << c.recordId << endl;
}

void case3() {
    string sn; int lvl;
    cout << "Skill Name: "; getline(cin, sn);
    cout << "Skill Level (1-10): "; cin >> lvl; cin.ignore();
    Skill s(sn, lvl);
    SkillSet.add(s);
    cout << "Skill added successfully with ID " << s.recordId << endl;
}

void case4() {
    displayCustomersTable();
    int cid;
    cout << "Enter Customer ID from above: "; cin >> cid;
    if (!customers.searchById(cid)) { cout << "Customer not found! Work order not added.\n"; cin.ignore(); return; }
    displayAssetsTable();
    if (assets.isEmpty()) { cout << "No assets available! Kindly add an Asset before creating a Work Order.\n"; cin.ignore(); return; }
    int aid;
    cout << "Enter Asset ID from above: "; cin >> aid;
    Asset *ast = assets.searchById(aid);
    if (!ast) { cout << "Asset not found! Work order not added.\n"; cin.ignore(); return; }
    string prio, stat;
    cout << "Priority (1. High 2. Medium 3. Low | default value = 'Low' ): ";
    int pr = -1; cin >> pr;
    if (pr == 1) prio = "High";
    else if (pr == 2) prio = "Medium";
    else if (pr == 3) prio = "Low";
    else prio = "Low";
    cout << "Status (1. Pending 2. Assigned 3. Completed): ";
    int st = -1; cin >> st;
    if (st == 1) stat = "Pending";
    else if (st == 2) stat = "Assigned";
    else if (st == 3) stat = "Completed";
    else stat = "Pending";
    WorkOrder wo(prio, stat, cid, aid);
    displaySkillsTable();
    cout << "Enter required skill IDs (space separated, end with -1): ";
    int sk;
    while (cin >> sk && sk != -1) {
        if (SkillSet.searchById(sk)) wo.requiredSkillSet.push_back(sk);
        else cout << "Skill ID " << sk << " not found, skipped.\n";
    }
    cin.ignore();
    workOrders.add(wo);
    cout << "Work Order added with ID " << wo.recordId << endl;
}

void case5() {
    displayUsersTable();
    int uid;
    cout << "Enter User ID (for this technician): "; cin >> uid;
    User *uList = users.searchById(uid);
    if (!uList) { cout << "User not found! Technician not added.\n"; cin.ignore(); return; }
    else if (uList->role != "technician") { cout << "User not found with role Technician.\n"; cin.ignore(); return; }
    ServiceResource *existingTech = NULL;
    Node<ServiceResource> *tcurr = technicians.getHead();
    while (tcurr) {
        if (tcurr->data.userId == uid) { existingTech = &tcurr->data; break; }
        tcurr = tcurr->next;
    }
    displaySkillsTable();
    cout << "Enter skill IDs for this technician (space separated, end -1): ";
    int sk;
    if (existingTech) {
        while (cin >> sk && sk != -1) {
            if (SkillSet.searchById(sk)) existingTech->skillSet.push_back(sk);
            else cout << "Skill ID " << sk << " not found, skipped.\n";
        }
        cin.ignore();
        cout << "Technician ID " << existingTech->recordId << " skills updated successfully.\n";
    }
    else {
        ServiceResource tech(uid);
        while (cin >> sk && sk != -1) {
            if (SkillSet.searchById(sk)) tech.skillSet.push_back(sk);
            else cout << "Skill ID " << sk << " not found, skipped.\n";
        }
        cin.ignore();
        technicians.add(tech);
        cout << "Technician added with ID " << tech.recordId << endl;
    }
}

void case6() {
    string pn;
    cout << "Product Name: "; getline(cin, pn);
    Asset a(pn);
    assets.add(a);
    cout << "Asset added successfully with ID " << a.recordId << endl;
}

void deleteRecordSubMenu() {
    cout << "\n=========================================\n";
    cout << "             DELETE RECORD               \n";
    cout << "=========================================\n";
    cout << "1. User\n";
    cout << "2. Customer\n";
    cout << "3. Skill\n";
    cout << "4. Work Order\n";
    cout << "5. Asset\n";
    cout << "6. Technician\n";
    cout << "7. Appointment\n";
    cout << "8. Return to Main Menu\n";
    cout << "=========================================\n";
    cout << "Select Entity to Delete: ";
    int subChoice;
    if (!(cin >> subChoice)) {
        cin.clear(); cin.ignore(1000, '\n');
        cout << "Invalid input! Returning to main menu.\n";
        return;
    }
    cin.ignore();
    switch (subChoice) {
    case 1: {
        displayUsersTable();
        if (users.isEmpty()) break;
        int id;
        cout << "Enter User ID to delete: ";
        if (!(cin >> id)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input!\n"; break; }
        cin.ignore();
        User *u = users.searchById(id);
        if (!u) { cout << "User ID " << id << " not found!\n"; break; }
        Node<ServiceResource> *currTech = technicians.getHead();
        while (currTech) {
            if (currTech->data.userId == id) {
                technicians.deleteById(currTech->data.recordId);
                break;
            }
            currTech = currTech->next;
        }
        users.deleteById(id);
        cout << "User ID " << id << " deleted successfully.\n";
        saveAll();
        break;
    }
    case 2: {
        displayCustomersTable();
        if (customers.isEmpty()) break;
        int id;
        cout << "Enter Customer ID to delete: ";
        if (!(cin >> id)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input!\n"; break; }
        cin.ignore();
        Customer *c = customers.searchById(id);
        if (!c) { cout << "Customer ID " << id << " not found!\n"; break; }
        customers.deleteById(id);
        cout << "Customer ID " << id << " and related Work Orders deleted successfully.\n";
        saveAll();
        break;
    }
    case 3: {
        displaySkillsTable();
        if (SkillSet.isEmpty()) break;
        int id;
        cout << "Enter Skill ID to delete: ";
        if (!(cin >> id)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input!\n"; break; }
        cin.ignore();
        Skill *s = SkillSet.searchById(id);
        if (!s) { cout << "Skill ID " << id << " not found!\n"; break; }
        SkillSet.deleteById(id);
        cout << "Skill ID " << id << " deleted successfully.\n";
        saveAll();
        break;
    }
    case 4: {
        displayWorkOrdersTable();
        if (workOrders.isEmpty()) break;
        int id;
        cout << "Enter Work Order ID to delete: ";
        if (!(cin >> id)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input!\n"; break; }
        cin.ignore();
        WorkOrder *w = workOrders.searchById(id);
        if (!w) { cout << "Work Order ID " << id << " not found!\n"; break; }
        workOrders.deleteById(id);
        cout << "Work Order ID " << id << " deleted successfully.\n";
        saveAll();
        break;
    }
    case 5: {
        displayAssetsTable();
        if (assets.isEmpty()) break;
        int id;
        cout << "Enter Asset ID to delete: ";
        if (!(cin >> id)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input!\n"; break; }
        cin.ignore();
        Asset *a = assets.searchById(id);
        if (!a) { cout << "Asset ID " << id << " not found!\n"; break; }
        assets.deleteById(id);
        cout << "Asset ID " << id << " deleted successfully.\n";
        saveAll();
        break;
    }
    case 6: {
        displayTechniciansTable();
        if (technicians.isEmpty()) break;
        int id;
        cout << "Enter Technician ID to delete: ";
        if (!(cin >> id)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input!\n"; break; }
        cin.ignore();
        ServiceResource *t = findTechnicianById(id);
        if (!t) { cout << "Technician ID " << id << " not found!\n"; break; }
        technicians.deleteById(t->recordId);
        cout << "Technician ID " << id << " deleted successfully.\n";
        saveAll();
        break;
    }
    case 7: {
        displayAppointmentsTable();
        if (appointments.isEmpty()) break;
        int id;
        cout << "Enter Appointment ID to delete: ";
        if (!(cin >> id)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input!\n"; break; }
        cin.ignore();
        Appointment *apt = appointments.searchById(id);
        if (!apt) { cout << "Appointment ID " << id << " not found!\n"; break; }
        ServiceResource *tech = technicians.searchById(apt->technicianId);
        if (tech && apt->slotIndex >= 0 && apt->slotIndex < (int)tech->slots.size()) {
            tech->slots[apt->slotIndex] = AVAILABLE;
        }
        WorkOrder *wo = workOrders.searchById(apt->workOrderId);
        if (wo) wo->status = "Pending";
        appointments.deleteById(id);
        cout << "Appointment ID " << id << " deleted successfully and slot freed.\n";
        saveAll();
        break;
    }
    case 8:
        cout << "Returning to main menu.\n";
        break;
    default:
        cout << "Invalid choice! Returning to main menu.\n";
    }
}

void printMenuOperations() {
    cout << "\n============================================================\n";
    cout << "               FIELD SERVICE DISPATCH SYSTEM                \n";
    cout << "============================================================\n\n";
    cout << " 1. Add User                       2. Add Customer\n";
    cout << " 3. Add Skill                      4. Add Work Order\n";
    cout << " 5. Add Technician                 6. Add Asset\n\n";
    cout << " 7. List Appointments              8. List Users\n";
    cout << " 9. List Customers                10. List Skills\n";
    cout << "11. List Work Orders              12. List Technicians\n";
    cout << "13. List Assets\n\n";
    cout << "14. Dispatch Work Orders          15. Create Appointment Manually\n";
    cout << "16. Complete Appointment (by ID)  17. Complete All Appointments\n";
    cout << "18. Block Technician's Slot       19. Add Sample Data\n\n";
    cout << "20. Show Technician Utilization   21. Show Pending Work Orders\n";
    cout << "22. Delete Record                 23. Save & Exit\n\n";
    cout << "============================================================\n";
    cout << "\nEnter Choice: ";
}

void menu() {
    int choice;
    do {
        printMenuOperations();
        cin >> choice;
        if (cin.fail()) {
            cin.clear(); cin.ignore(1000, '\n');
            cout << "Invalid choice! Please enter a valid choice number.\n";
            continue;
        }
        cin.ignore();
        switch (choice) {
        case 1: case1(); break;
        case 2: case2(); break;
        case 3: case3(); break;
        case 4: case4(); break;
        case 5: case5(); break;
        case 6: case6(); break;
        case 7: displayAppointmentsTable(); break;
        case 8: displayUsersTable(); break;
        case 9: displayCustomersTable(); break;
        case 10: displaySkillsTable(); break;
        case 11: displayWorkOrdersTable(); break;
        case 12: displayTechniciansTable(); break;
        case 13: displayAssetsTable(); break;
        case 14: dispatchAll(); break;
        case 15: createAppointment(); break;
        case 16: {
            displayAppointmentsTable();
            if (appointments.isEmpty()) break;
            int aptId;
            cout << "Enter Appointment ID: ";
            if (!(cin >> aptId)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input! Returning to main menu.\n"; break; }
            cin.ignore();
            completeAppointment(aptId);
            break;
        }
        case 17: completeAllAppointments(); break;
        case 18: {
            displayTechniciansTable();
            if (technicians.isEmpty()) break;
            int techId;
            cout << "Enter Technician ID: ";
            if (!(cin >> techId)) { cin.clear(); cin.ignore(1000, '\n'); cout << "Invalid input! Returning to main menu.\n"; break; }
            cin.ignore();
            ServiceResource *tech = findTechnicianById(techId);
            if (tech) tech->blockTechnicianAvailableSlot();
            else cout << "Technician ID " << techId << " not found! Returning to main menu.\n";
            break;
        }
        case 19: addSampleData(); break;
        case 20: showUtilization(); break;
        case 21: showPendingWorkOrders(); break;
        case 22: deleteRecordSubMenu(); break;
        case 23:
            isProgramExiting = true;
            saveAll();
            cout << "Exiting...\n";
            break;
        default: cout << "Invalid choice.\n";
        }
    } while (choice != 23);
}

int main() {
    createDataDir();
    loadAll();
    menu();
    return 0;
}