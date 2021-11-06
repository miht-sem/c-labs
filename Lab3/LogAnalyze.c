#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static FILE *inFile;
static int isLineLast;

char *readStringOfLog(int lenOfString)
{
	int symbol = getc(inFile);
	char *stringOfLog;

	if (symbol != '\n' && symbol != EOF)
	{
		stringOfLog = readStringOfLog(lenOfString + 1);
		stringOfLog[lenOfString] = symbol;
	}
	else
	{
		stringOfLog = (char *)realloc(stringOfLog, lenOfString + 1);
		stringOfLog[lenOfString] = '\0';
		isLineLast = (symbol == EOF ? 1 : 0);
	}
	return stringOfLog;
}

char *logParser(char **stringOfTime, char **stringOfRequest, char **stringOfServerStatus)
{
	char *stringOfLog = readStringOfLog(0);
	char *tempChar;

	strtok_r(stringOfLog, "[", &tempChar);
	*stringOfTime = strtok_r(NULL, "]", &tempChar);
	strtok_r(NULL, "\"", &tempChar);
	*stringOfRequest = strtok_r(NULL, "\"", &tempChar);
	*stringOfServerStatus = strtok_r(NULL, " ", &tempChar);
	return stringOfLog;
}

int logTimeParser(char *stringOfTime)
{
	struct tm time;
	char *tempChar;
	char *curString;

	curString = strtok_r(stringOfTime, "/", &tempChar);
	time.tm_mday = atoi(curString);

	curString = strtok_r(NULL, "/", &tempChar);

	const char *arrayOfMonthes[12] =
		{"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	for (int numberOfMonth = 0; numberOfMonth < 12; numberOfMonth++)
	{
		if (!strcmp(curString, arrayOfMonthes[numberOfMonth]))
			time.tm_mon = numberOfMonth;
	}

	curString = strtok_r(NULL, ":", &tempChar);
	time.tm_year = atoi(curString) - 1900;

	curString = strtok_r(NULL, ":", &tempChar);
	time.tm_hour = atoi(curString);

	curString = strtok_r(NULL, ":", &tempChar);
	time.tm_min = atoi(curString);

	curString = strtok_r(NULL, " ", &tempChar);
	time.tm_sec = atoi(curString);

	return mktime(&time);
}

int transformInSeconds(char *timeType)
{
	if (!strcmp(timeType, "sec") || !strcmp(timeType, "secs"))
		return 1;
	else if (!strcmp(timeType, "min") || !strcmp(timeType, "mins"))
		return 60;
	else if (!strcmp(timeType, "hour") || !strcmp(timeType, "hours"))
		return 3600;
	else if (!strcmp(timeType, "day") || !strcmp(timeType, "days"))
		return 3600 * 24;
	else
		return 0;
}

int neededPeriod()
{
	char neededPeriodOfTime[100];
	char *tempChar;

	printf("Input time period using hour/hours, day/days,\nhour/hours, sec/secs : ");
	scanf("%99[^\n]", neededPeriodOfTime);

	char *curString;
	int number, typeOfTime;

	curString = strtok_r(neededPeriodOfTime, " ", &tempChar);
	if (curString == NULL)
		return -1;
	int periodInSeconds = 0;

	while (curString != NULL)
	{
		number = atoi(curString);

		curString = strtok_r(NULL, " ", &tempChar);
		if (curString == NULL)
			return -1;

		typeOfTime = transformInSeconds(curString);
		if (typeOfTime == 0)
			return -1;

		periodInSeconds += number * typeOfTime;
		curString = strtok_r(NULL, " ", &tempChar);
	}
	return periodInSeconds;
}

typedef struct Request
{
	int curTime;
	int requestNumber;
	struct Request *previous;
	struct Request *next;
} Request;

typedef struct ListOfRequests
{
	Request *first;
	Request *last;
} ListOfRequests;

void push(ListOfRequests *listOfRequests, int timeOfRequest, int requestNumber)
{
	Request *pushElement = (Request *)calloc(1, sizeof(Request));
	pushElement->curTime = timeOfRequest;
	pushElement->requestNumber = requestNumber;
	pushElement->previous = pushElement->next = NULL;

	if (listOfRequests->first == NULL)
		listOfRequests->last = pushElement;
	else
	{
		pushElement->next = listOfRequests->first;
		listOfRequests->first->previous = pushElement;
	}
	listOfRequests->first = pushElement;
}

void pop(ListOfRequests *listOfRequests)
{
	Request *popElement = listOfRequests->last;
	if (popElement == NULL)
		return;
	if (popElement->previous != NULL)
	{
		listOfRequests->last = popElement->previous;
		listOfRequests->last->next = NULL;
	}
	else
		listOfRequests->first = listOfRequests->last = NULL;
	free(popElement);
}

int main()
{
	inFile = fopen("access_log_Jul95.txt", "r");

	int timePeriod = neededPeriod();
	if (timePeriod == -1)
	{
		printf("Incorrect input.\n");
		return 1;
	}
	else if (timePeriod == 0)
	{
		printf("Can't be 0 sec period.\n");
		return 1;
	}

	int timeOfRequest;
	int badRequest = 0;
	isLineLast = 0;

	char *stringOfTime;
	char *stringOfRequest;
	char *stringOfServerStatus;
	char *stringTemp;

	int maxRequests;
	int startTime;
	int endTime;

	ListOfRequests listOfRequests = {NULL, NULL};

	printf("\nBad requests:\n");

	for (int numberOfLine = 0; !isLineLast; numberOfLine++)
	{
		stringTemp = logParser(&stringOfTime, &stringOfRequest, &stringOfServerStatus);
		
		if (stringOfTime != NULL)
		{
			timeOfRequest = logTimeParser(stringOfTime);
			push(&listOfRequests, timeOfRequest, numberOfLine);

			while (timeOfRequest - listOfRequests.last->curTime > timePeriod)
				pop(&listOfRequests);

			int numberOfRequests = numberOfLine - listOfRequests.last->requestNumber + 1;

			if (numberOfRequests > maxRequests)
			{
				maxRequests = numberOfRequests;
				startTime = listOfRequests.last->curTime;
				endTime = timeOfRequest;
			}
		}

		if (stringOfServerStatus != NULL && stringOfServerStatus[0] == '5')
		{
			printf("%s \n", stringOfRequest);
			badRequest++;
		}

		free(stringTemp);
	}

	printf("\nTotal amount of bad requests: %d", badRequest);
	printf("\nThe largest amount of requests in the needed period: %d\n", maxRequests);

	time_t timeInSeconds;
	char *time;

	timeInSeconds = startTime;
	time = asctime(localtime(&timeInSeconds));
	printf("\nStart of needed period: %s", time);

	timeInSeconds = endTime;
	time = asctime(localtime(&timeInSeconds));
	printf("End of needed period: %s\n", time);

	fclose(inFile);
	return 0;
}
