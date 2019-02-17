#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

pid_t father;
int size = 0;
int num = 0;
pid_t *pids = NULL;

/*
 * Для выделения памяти для массива пидов
 */
pid_t* allocate_pids (pid_t* pids, int size)
{
    pid_t *tmp = NULL;
    tmp = (pid_t *)realloc(pids, sizeof(pid_t) * (size + 1));
    if (tmp == NULL)
    {
        perror(NULL);
        if (pids != NULL)
            free(pids);
        return NULL;
    }
    pids = tmp;
    return pids;
}

void end_cycle_handler(int signum)
{
    if (getpid() == father)
    {
        signal(signum, end_cycle_handler);
        size--;
        kill(pids[size - 1], SIGUSR2);
        while(wait(NULL) != -1);
        free(pids);
        exit(0);
    }
}

void output_handler (int signum)
{
    signal(signum, output_handler);
    printf("%d\n", num);
    if (size - 1 >= 0)
        kill(pids[size - 1], signum);
    free(pids);
    exit(0);
}

void handler (int signum)
{
    /* Костыль
     * Для ожидания сыночков, чтобы те прочитали число
     * Хз как сделать это получше чем пустой обработчик :DDD
     */
}

int main ()
{
    father = getpid();
    signal(SIGUSR1, end_cycle_handler);
    signal(SIGCONT, handler);
    while (1)
    {
        pids = allocate_pids(pids, size);
        if (pids == NULL)
        {
            printf("-1\n");
            return 1;
        }
        pids[size] = fork();
        if (!pids[size])
        {
            pids[size] = getpid();
            signal(SIGUSR2, output_handler);
            if (scanf("%d", &num) == EOF)
            {
                /*
                 * Если конец файла, то кидаем сигнал в отца и ливаем
                 */
                kill(father, SIGCONT);
                kill(father, SIGUSR1);
                return 0;
            }
            kill(father, SIGCONT);
            /*
             * А здесь сыночек ждет на паузе
             * И ждет своей очереди, чтобы вывести число
             */
            pause();
            return 0;
        }
        else if (pids[size] == -1)
        {
            printf("-1\n");
            return 1;
        }
        size++;
        /*
         * Висим на паузе и ждем когда сыночек прочитает число
         * Иначе эти сыночки будут очень быстро и вечно плодиться
         * И будет жёпа
         */
        pause();
    }
    return 2;

}
