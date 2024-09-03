#include <iostream>
#include <sstream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <random>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>   
#include <vector>    
#include <iterator>  
#include <chrono>

std::string typs[] = {"SPORTS", "NEWS", "WEATHER"}; //for the typs of srings
class UnBounded_Buffer{
        std::mutex mtx; //mutex
        int count; //how many strings are in this buffer now
        std::queue<std::string> buffer; 
        public:
        std::condition_variable not_empty;
        //Constructor
        UnBounded_Buffer(){count = 0;}

        void insert(std::string add) {
            std::unique_lock<std::mutex> lock(mtx); //get a mutex that we will try to close later
                buffer.push(add);
                count++; //one more in
                not_empty.notify_one();  //somthing of semphore that wake I thing other thread
         }
          std::string remove() {
            std::unique_lock<std::mutex> lock(mtx); //get a mutex that we will try to close later
            /*
            if the locker not close with other thread and the buffer is not empty
            */
            not_empty.wait(lock, [this]() { return count != 0; });
            std::string head = buffer.front();
            buffer.pop();
            count--;
            return head; //return the first elemnt
         }
         int getCount(){
            return count;
         }







};
/*
class for buffer of strings
*/
class Bounded_Buffer{
    private:
        std::mutex mtx; //mutex
        int size; //size of cournt buffer that we got in the confgrtion
        int count; //how muce string in this buffer now
        std::queue<std::string> buffer; 
        std::condition_variable not_full; //somthing that helo to semefore
    public:
        std::condition_variable not_empty; //somthing that helo to semefore
            //constucter
         Bounded_Buffer(int sizeToUpdete) : size(sizeToUpdete), count(0) {}
         void insert(std::string add) {
            std::unique_lock<std::mutex> lock(mtx); //get a mutex that we will try to close later
            /*
            if the locker not close with other thread and the buffer is not full
            */
            not_full.wait(lock, [this]() { return count != size; }); 
                
                buffer.push(add);
                count++; //one more in
                not_empty.notify_one();  //somthing of semphore that wake I thing other thread
         }
         std::string remove() {
            std::unique_lock<std::mutex> lock(mtx); //get a mutex that we will try to close later
            /*
            if the locker not close with other thread and the buffer is not empty
            */
            not_empty.wait(lock, [this]() { return count != 0; });
            std::string head = buffer.front();
            buffer.pop();
            count--;
            not_full.notify_one();  //somthing of semphore that wake I thing other thread
            return head; //return the first elemnt
         }
         int getCount(){
            return count;
         }
};
/*
producer that get the buffer to put in and the ID of it and how much product to make
*/
void Producer(Bounded_Buffer &buf, int therdID, int numberOfPrudoct){
    //random of c++
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);
    int counter[]={0, 0, 0}; //counter for how much string from specfic type we did
    for(int i=0; i<numberOfPrudoct; i++) {
        int randomIndex = dis(gen);  //random numer for the type to put in the string
        std::string message = "Producer";
        message += " " + std::to_string(therdID);
        message += " " + typs[randomIndex]; //type from the globel array of typs
        message += " " + std::to_string(counter[randomIndex]);
        buf.insert(message); //push the string to the buffer
        counter[randomIndex]++; //one more from this type
    }
    buf.insert("Done"); //after finsh the numer that I have to put
}
/*
Dispatcher get array of typs to put in it, array of prodrcers buffers and the number of them
*/
void Dispatcher(UnBounded_Buffer** typsBuffer, Bounded_Buffer** producerBuffers, int numProcducers){
    bool finish[numProcducers]; //for flag whan procducer sent DONE
    int countTofinish=numProcducers;
    for(int i = 0; i < numProcducers; i++) {
        finish[i]=false; //initialize
    }
    while (countTofinish) //while ther is thread that dont finish
    {
        for (int i = 0; i < numProcducers; i++) {
        if(!finish[i]) {
            // check if producer's buffer is empty. if so - skip current producer.
            if(producerBuffers[i]->getCount()>0){
                std::string prod=producerBuffers[i]->remove();
                if(prod == "Done") {
                        finish[i]=true;
                        countTofinish--;
                    } else {
                        std::istringstream iss(prod);
                        std::string type;
                        // Ignore the first two words
                        iss >> type; // first word
                        iss >> type; // second word
                        // Now extract the third word
                        iss >> type;
                        /*
                        put the string in the match buffer
                        */
                        if(type == typs[0]) {
                            typsBuffer[0]->insert(prod);
                        } else if (type == typs[1])
                        {
                            typsBuffer[1]->insert(prod);
                        } else {
                            typsBuffer[2]->insert(prod);
                        }
                    } 
            }
          
        }
      }
    } 
         typsBuffer[0] -> insert("Done"); 
         typsBuffer[1] -> insert("Done"); 
         typsBuffer[2] -> insert("Done"); 


    
}
/*
for you function of coEtidor
 */
void coEditor(UnBounded_Buffer *coBuff, UnBounded_Buffer *screenBuf) {
    bool finish = 1; //for flag when buffer's type sent DONE
    int DoneCount = 0;
    std::chrono::duration<double> duration(0.1);
    while(finish){
        std::string prod=coBuff->remove();
        if(prod == "Done"){
          screenBuf->insert("Done");
          DoneCount++;
          return;
        }
        else{
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(duration));
            screenBuf->insert(prod);
        }
    }
    return;
}
/*
for you function of screen
 */
void screen(UnBounded_Buffer *coBuff) {
    bool finish = 1; //for flag when coEditor sent DONE
    int DoneCount = 0;
    while(finish){
        std::string prod=coBuff->remove();
        if(prod=="Done"){
           DoneCount++;
           if(DoneCount==3){
            finish = 0;
           }
        }
        else{
            std::cout<<prod<<std::endl;
        }
    }
    std::cout<<"Done"<<std::endl;
    return;
}
/**
 *  get number of producers in config file
*/
int getNumberOfProducers(char* configFileName){
    std::ifstream file;
    file.open(configFileName);
    std::string line;
    int n_line = 0;
    while(std::getline(file, line)){
        ++n_line;
    }
    int n_producers = n_line/4;
    file.seekg(0, std::ios::beg); // Set the position to the beginning
    file.close();
    return n_producers;
}
/**
 * get number of messeges from the i'th producers
*/
int getNumberOfMessages(int numberOfProducer, char* configFileName){
    std::ifstream file;
    file.open((configFileName));
    std::string line;
    // get line under current producer's title in config file
    int n_line = 0;
    int current_line = 2 + numberOfProducer*4;
    while(++n_line!= current_line){
        std::getline(file, line);
    }
    std::getline(file, line);
    int n_meggages = std::stoi(line);
    file.seekg(0, std::ios::beg); // Set the position to the beginning
    file.close();
    return n_meggages;
}
/**
 * get size of queue of specific producer from confid file
*/
int getQueueSize(int numberOfProducer, char* configFileName){
    std::ifstream file;
    file.open((configFileName));
    std::string line;
    // get line under current producer's title in config file
    int n_line = 0;
    int current_line = 3 + numberOfProducer*4;
    while(++n_line!= current_line){
        std::getline(file, line);
    }
    std::getline(file, line);
    std::istringstream iss(line);
    std::vector <std::string> words{
        std::istream_iterator<std::string>{iss},
        std::istream_iterator<std::string>{}
    };
    int queue_size = std::stoi(words[3]);
    file.seekg(0, std::ios::beg); // Set the position to the beginning
    file.close();
    return queue_size;
}


int main(int argc, char* argv[]){

     // Check if the correct number of arguments is provided
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    int n_producers = getNumberOfProducers(argv[1]);
    Bounded_Buffer* producerBuffers[n_producers];
    int timesForEveryProducer[n_producers];
    for (int i = 0; i < n_producers; i++) {
        //initialize before I read from the configrition
        producerBuffers[i] = new Bounded_Buffer(getQueueSize(i, argv[1]));
        timesForEveryProducer[i]= getNumberOfMessages(i, argv[1]);
    }
    std::vector<std::thread> threads(n_producers+1); //keep all the thred here
    for (int i = 0; i < n_producers; i++) {
        //make prodecres
        threads[i] = std::thread(Producer,std::ref(*producerBuffers[i]), i+1, timesForEveryProducer[i]);
    }
    UnBounded_Buffer* typsBuffers[3];
    for(int i=0; i<3; i++) {
        typsBuffers[i] = new UnBounded_Buffer(); 
    }
    //thread for the dispatcher
    threads[n_producers] =std::thread(Dispatcher, (UnBounded_Buffer**)typsBuffers, (Bounded_Buffer**)producerBuffers,n_producers);
   
    
    
    std::vector<std::thread> threadsMore(4); //keep all the thred here
    UnBounded_Buffer* screenBuffer= new UnBounded_Buffer(); //for screen we need to chnge the size acording to configoriton
    //thread for coEditor
    for(int i=0; i<3; i++) {
        threadsMore[i] =std::thread(coEditor, typsBuffers[i], screenBuffer);
    }
    threadsMore[3] =std::thread(screen, screenBuffer);
    

    /*
    the main procces wait for threads to finish 
    */

    for (auto& th : threads) {
             th.join();
    }
    for (auto& th : threadsMore) {
             th.join();
    }


    

   
}