#include <iostream>

int main(){
    //printing hello ( question 1)
    std::string message  = "hello"; 
    std::cout << message << std::endl ;
    //getting input and printing it 
    std::string input = "";
    std::cin >> input;
    std::cout << input << std::endl;
    //addition question ( question 2)
    std::cout << "Enter two integers to add" << std::endl;
    int num1, num2;
    std::cin >> num1;
    std::cin >> num2;
    int sum = num1 + num2;
    std::cout << sum << std::endl;

    //question 3. check even or odd 
    std::cout << "question 3. input a number" << std::endl;
    int evenOrOdd;
    std::cin >> evenOrOdd;
    if (evenOrOdd % 2 == 1){
        std::cout << "Odd number" << std::endl;
    }else{
        std::cout << "even number" << std::endl;
    }

    //question 6. 1 to n
    std::cout << "input a number n to loop to " << std::endl;
    int n;
    std::cin >> n;
    for (int i = 0; i < abs(n); i ++){
        std::cout<< (i+1) << std::endl;
    }

    //question 7 factorial
    std::cout << "input a number to calculate the factorial of " << std::endl;
    int f;
    std::cin >> f;
    int factorial = 1;
    for (int i = 0; i < f; i++){
        factorial = (i+1) * factorial;
    }
    std::cout << factorial << std::endl;


    //question 10. reverse a string 
    std::cout << "input a string to reverse it " << std::endl;
    std::string strang; 
    std::cin >> strang;
    std::string reversed;
    for (int i = strang.size() ; i > 0; i--){
        reversed += (strang.at(i-1));
    }
    std::cout << reversed << std::endl;
    return 0;
}