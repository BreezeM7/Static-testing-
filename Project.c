#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include<stdlib.h>
#include <time.h>

//cupcake structure
struct CupCake {
    char flavor[12];
    int cupCakeNumber;
    int bakerID;
    int consumerID;
};

//initilization of important variables
int numOfBakers;
int traySize;
int numOfCustomers;
int weekend;
int numOfConsumedCupCakes = 0;
int numOfProducedCupCakes = 0;
int producerArrayIndex = 0;
int consumerArrayIndex = 0;
int numOfAllowedCupCakes = 0;
int usedArraySize = 0;

//initilization of cupcake array pointers 
//one array for the tray and the other for all cupcakes to be saved in for output
struct CupCake* cupCakesTrayArray;
struct CupCake* allCupCakesProducedArray;

//arrays to store the number of cupcakes produced
//or cupcakes consumed per each customer and consumer
int* producerCounts;
int* customerCounts;

//creating 200 cupcake structers
struct CupCake CupCake[200];

//initilizing an empty cupcake
struct CupCake emptyCupcake = {"", -1, -1, -1};

//flavor array that includes all flavors to be randomly chosen from later
const char* flavorsArray[] = {"Chocolate", "Orange", "Vanilla", "Strawberry"};

//an array of characters that will include all of the output
char allCupCakes[50000];

//creating the mutex thread for thread synchronization

//mutex for baker threads
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

//mutex for customer threads
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

//mutex for both threads
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;



//baker method
void bakerProduce(){

    //an infinite loop
    while (1){
        
        //locking the threads to make sure baker threads are in sync
        pthread_mutex_lock(&mutex1);
        
        //making sure the number of produced cupcakes doesnt exceed the allowed
        if (numOfProducedCupCakes >= numOfAllowedCupCakes){
            pthread_mutex_unlock(&mutex1);
            return;
        }

        //making sure tray is not full
        while(1){
            if(usedArraySize >= traySize){
            continue;
            };
            break;
        };

        //locking mutex between baker thread and customer thread
        pthread_mutex_lock(&mutex3);

        //making sure to start from the beggening if the array is filled
        if (producerArrayIndex >= traySize){
            producerArrayIndex = 0;
        }

        //produces the cupcake
        createCupCake();
        
        //printing information about the produced cupcake and the baker
        printf("Thread ID: %lu produced a CupCake\n", (unsigned long)pthread_self());
        printf("number of cupcakes in tray %d\n", usedArraySize+1);
        printf("array index: %d\n", producerArrayIndex);
        printf("number of produced cupcakes: %d\n", numOfProducedCupCakes+1);
        printf("CupCake produced: \n");
        cupcakePrint(numOfProducedCupCakes);
        printf("-----------------------------------------\n");

        //incrementing the variables
        usedArraySize++;
        numOfProducedCupCakes++;
        producerArrayIndex++;
        producerCounts[(unsigned long)pthread_self() - 1]++;

        //waiting for one second
        sleep(1);
        pthread_mutex_unlock(&mutex3);

        //unlocking the section so other threads can enter
        pthread_mutex_unlock(&mutex1);

        //simply starting a new line
        printf("\n");
    };
};



//customer method
void customerConsume(){
    
    while (1){

        //locking the threads to make sure customer threads are in sync
        pthread_mutex_lock(&mutex2);

        //making sure the number of consumed cupcakes doesnt exceed the allowed
        if (numOfConsumedCupCakes >= numOfAllowedCupCakes){
            pthread_mutex_unlock(&mutex2);
            return;
        };

        //to make sure that cupcakes are avalible in tray
        while(1){
            if(usedArraySize < 2){
            continue;
            };
            break;
        };

        //locking mutex between baker thread and customer thread
        pthread_mutex_lock(&mutex3);

        //calling the consume cupcake twise since
        //each customer should consume 2 cupcakes
        consumeCupCake();
        consumeCupCake();
        pthread_mutex_unlock(&mutex3);

        //unlocking the section so other threads can enter
        pthread_mutex_unlock(&mutex2);

        //simply starting a new line
        printf("\n");

    }
    
};



//consumes a cupcake from the array
void consumeCupCake(){

    //making sure the number of consumed cupcakes doesnt exceed the allowed
    if (numOfConsumedCupCakes >= numOfAllowedCupCakes){
        return;
    };

    //making sure to start from the beggening if the array is consumed
    if (consumerArrayIndex >= traySize){
        consumerArrayIndex = 0;
    };

    //consuming cupcake from tray
    //and assigning the consumer to it in all cupcake array
    cupCakesTrayArray[consumerArrayIndex] = emptyCupcake;
    allCupCakesProducedArray[numOfConsumedCupCakes].consumerID = (unsigned long)pthread_self();
    

    //printing information about the consumed cupcake and the consumer
    printf("Thread ID: %lu consumed a CupCake\n", (unsigned long)pthread_self());
    printf("number of cupcakes in tray %d\n", usedArraySize-1);
    printf("array index: %d\n", consumerArrayIndex);
    printf("number of consumed cupcakes: %d\n", numOfConsumedCupCakes+1);
    cupcakePrint(numOfConsumedCupCakes);
    printf("-----------------------------------------\n");

    //incrementing the variables
    consumerArrayIndex++;
    usedArraySize--;
    numOfConsumedCupCakes++;
    customerCounts[((unsigned long)pthread_self() - numOfBakers)-1]++;

    //waiting for one second
    sleep(1);
};



//print cupcake struct values
void cupcakePrint(int index){
    printf("CupCake %d:\n", index);
    printf("Flavor: %s\n", allCupCakesProducedArray[index].flavor);
    printf("CupCake Number: %d\n", allCupCakesProducedArray[index].cupCakeNumber);
    printf("CupCake BakerID: %d\n", allCupCakesProducedArray[index].bakerID);
    printf("CupCake ConsumerID: %d\n", allCupCakesProducedArray[index].consumerID);
    printf("\n");
};



//method for creating a cupcake
void createCupCake(){
    //seeding the random number genirator the current time
    srand((unsigned int)time(NULL));

    //assigning values to the new cupcake as well as saving it in both arrays
    strcpy(CupCake[numOfProducedCupCakes].flavor, flavorsArray[rand()%4]);
    CupCake[numOfProducedCupCakes].cupCakeNumber = rand() % numOfAllowedCupCakes;
    CupCake[numOfProducedCupCakes].bakerID = (unsigned long)pthread_self();
    CupCake[numOfProducedCupCakes].consumerID = NULL;
    cupCakesTrayArray[producerArrayIndex] = CupCake[numOfProducedCupCakes];
    allCupCakesProducedArray[numOfProducedCupCakes] = CupCake[numOfProducedCupCakes];
};



//allocating memory for the arrays
void initializeArrays() {
    cupCakesTrayArray = (struct CupCake*)malloc(traySize * sizeof(struct CupCake));
    allCupCakesProducedArray = (struct CupCake*)malloc(numOfAllowedCupCakes * sizeof(struct CupCake));
};



//cleaning up the memory of the dinamically allocated arrays
void cleanupArrays() {
    free(cupCakesTrayArray);
    free(allCupCakesProducedArray);
};



int main(){

    int userChoice;
    //user input code
    printf("Please choose whether to read input interactively or from a file: \n");
    printf("(0) interactively\n(1) Reading from a file\n");
    scanf("%d", &userChoice);

    //choice false input prevention
    if (userChoice>1 || userChoice < 0){
        printf("invalid choice value\n");
        exit(0);
    };

    //taking user input from a file or interactively
    //file input format:
    // # Number of bakers
    // # Number of customers
    // # Tray size
    // # Weekend (0 for weekday, 1 for weekend)
    if (userChoice==1){
        readInputFromFile();
    } else if (weekend == 0){
        userInputInteractively();
    };

    //weekend false input prevention
    if (weekend>1 || weekend < 0){
        printf("invalid weekend value\n");
        exit(0);
    };

    //baker false input prevention
    if (numOfBakers <= 0){
        printf("invalid baker value\n");
        exit(0);
    };

    //customer false input prevention
    if (numOfCustomers <= 0){
        printf("invalid customer value\n");
        exit(0);
    };

    //tray false input prevention
    if (traySize <= 5){
        printf("invalid tray size value\n");
        exit(0);
    };

    //defining the number of allowed cupcakes for weekend or weekday
    if (weekend==1){
        numOfAllowedCupCakes = 200;
    } else if (weekend == 0){
        numOfAllowedCupCakes = 100;
    };

    //array memory allocation
    initializeArrays();

    //making all threads for bakers and consumers
    pthread_t thread[numOfBakers+numOfCustomers];

    //method for initilizing the count arrays
    countCupCakes();


    //bakers thread creation loop
    for (int i = 0; i<numOfBakers; i++){
        pthread_create (&thread[i], NULL, bakerProduce, NULL);
    };

    //Customer thread creation loop
    for (int i = numOfBakers; i < numOfBakers + numOfCustomers; i++){
        pthread_create (&thread[i], NULL, customerConsume, NULL);
    };

    //threads wait loop
    for (int i = 0; i<numOfCustomers+numOfBakers; i++){
        pthread_join(thread[i], NULL);
    };

    //printing all cupcakes information + who produced each and who consumed each
    printf("all CupCakes produced and consumed: \n");
    for (int i = 0; i<numOfAllowedCupCakes; i++){
        cupcakePrint(i);
    };

    //printing all the output to a file
    strcpy(allCupCakes, "");
    for (int i = 0; i < numOfAllowedCupCakes; i++){
        outputFileWrite(i);
    }

    //method for printing the count output and adding it to output file
    printProducerCustomerCounts();

    //adding all the output to the output file
    FILE *PF = fopen("ProjectOutputFile.txt", "w");
    if (PF == NULL) {
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }

    fprintf(PF, "%s", allCupCakes);
    fclose(PF);

    cleanupArrays();

    return 0;
};



//writing all cupcakes information in allcupcakes varaible
void outputFileWrite(int index){
    //putting all cupcakes in the allCupCakes variable to add to the output file later
    snprintf(allCupCakes + strlen(allCupCakes), sizeof(allCupCakes) - strlen(allCupCakes),
             "\nCupCake %d:\n"
             "Flavor: %s\n"
             "CupCake Number: %d\n"
             "CupCake BakerID: %d\n"
             "CupCake ConsumerID: %d\n"
             "---------------------------------------------------- \n\n",
             index,
             allCupCakesProducedArray[index].flavor,
             allCupCakesProducedArray[index].cupCakeNumber,
             allCupCakesProducedArray[index].bakerID,
             allCupCakesProducedArray[index].consumerID);
}



void readInputFromFile() {
    char fileName[100];

    //user input for the input file name
    printf("Enter the input file name: ");
    scanf("%s", fileName);

    FILE* file = fopen(fileName, "r");

    if (file == NULL) {
        perror("Error opening the file");
        exit(EXIT_FAILURE);
    }

    //read number of bakers value
    fscanf(file, "%d", &numOfBakers);

    //read number of customers value
    fscanf(file, "%d", &numOfCustomers);

    //read tray size value
    fscanf(file, "%d", &traySize);

    //read weekend value
    fscanf(file, "%d", &weekend);

    fclose(file);

    //print the values read from the file
    printf("Number of Bakers: %d\n", numOfBakers);
    printf("Number of Customers: %d\n", numOfCustomers);
    printf("Tray Size: %d\n", traySize);
    printf("Weekend: %d\n", weekend);
    printf("\n");
}


//input from the user interactivly
void userInputInteractively(){
    printf("Enter number of bakers\n");
    scanf("%d", &numOfBakers);
    printf("Enter number of customers\n");
    scanf("%d", &numOfCustomers);
    printf("Enter the size of the tray\n");
    scanf("%d", &traySize);
    printf("Enter the value of weekend\n(0) weekday\n(1) weekend\n");
    scanf("%d", &weekend);
    printf("\n");
}



//method for counting the cupcakes for producers and customers
void countCupCakes(){
    //initializing arrays to store counts
    producerCounts = (int*)malloc(numOfBakers * sizeof(int));
    customerCounts = (int*)malloc(numOfCustomers * sizeof(int));

    //initializing counts to zero
    for (int i = 0; i < numOfBakers; i++) {
        producerCounts[i] = 0;
    }

    for (int i = 0; i < numOfCustomers; i++) {
        customerCounts[i] = 0;
    }
}

//method for counting the cupcakes for producers and customers and printing them
//as well as adding them to the output variable allcupcakes
void printProducerCustomerCounts() {

    printf("\nNumber of cupcakes produced by each producer:\n");
    snprintf(allCupCakes + strlen(allCupCakes), sizeof(allCupCakes) - strlen(allCupCakes),
        "\nNumber of cupcakes produced by each producer:\n");
    for (int i = 0; i < numOfBakers; i++) {
        printf("Baker %d: %d cupcakes\n", i+1, producerCounts[i]);
        snprintf(allCupCakes + strlen(allCupCakes), sizeof(allCupCakes) - strlen(allCupCakes),
        "Baker %d: %d cupcakes\n", i+1, producerCounts[i]);
    };

    printf("\nNumber of cupcakes consumed by each customer:\n");
    snprintf(allCupCakes + strlen(allCupCakes), sizeof(allCupCakes) - strlen(allCupCakes),
        "\nNumber of cupcakes consumed by each customer:\n");
    for (int i = 0; i < numOfCustomers; i++) {
        printf("Customer %d: %d cupcakes\n", i+1, customerCounts[i]);
        snprintf(allCupCakes + strlen(allCupCakes), sizeof(allCupCakes) - strlen(allCupCakes),
        "Customer %d: %d cupcakes\n", i+1, customerCounts[i]);
    };

    //free allocated memory for counts arrays to prevent memory leaks
    free(producerCounts);
    free(customerCounts);
}