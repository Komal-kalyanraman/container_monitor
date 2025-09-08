#include <iostream>
#include <cstdint>
#include <cstring>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#pragma pack(push, 1)
struct ContainerMaxMetricsMsg {
    double max_cpu_usage_percent;
    double max_memory_usage_percent;
    double max_pids_percent;
    char container_id[100];
};
#pragma pack(pop)

int main() {
    std::cout << "sizeof(ContainerMaxMetricsMsg): " << sizeof(ContainerMaxMetricsMsg) << std::endl;

    // Fill struct with sample data
    ContainerMaxMetricsMsg msg;
    msg.max_cpu_usage_percent = 32.34;
    msg.max_memory_usage_percent = 66.78;
    msg.max_pids_percent = 70.12;
    strncpy(msg.container_id, "container_124", sizeof(msg.container_id));
    msg.container_id[sizeof(msg.container_id)-1] = '\0'; // Ensure null-termination
    std::cout << "Struct populated with sample data." << std::endl;

    // Message queue setup
    const char* queue_name = "/container_max_metric_mq";
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(ContainerMaxMetricsMsg);
    attr.mq_curmsgs = 0;

    std::cout << "Opening message queue: " << queue_name << std::endl;
    mqd_t mqd = mq_open(queue_name, O_CREAT | O_WRONLY, 0644, &attr);
    if (mqd == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }
    std::cout << "Message queue opened successfully." << std::endl;

    // Send message
    std::cout << "Sending message to queue..." << std::endl;
    if (mq_send(mqd, reinterpret_cast<const char*>(&msg), sizeof(msg), 0) == -1) {
        perror("mq_send");
        mq_close(mqd);
        mq_unlink(queue_name);
        return 1;
    }
    std::cout << "Message sent to queue." << std::endl;

    mq_close(mqd);
    // mq_unlink(queue_name);
    std::cout << "Message queue closed and unlinked." << std::endl;
    return 0;
}