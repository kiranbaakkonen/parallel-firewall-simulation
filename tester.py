import subprocess
import sys
import numpy as np
import os

def correctness_test():
    print("Running correctness test")
    sum = 0
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "200", "1", "0"])
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "400", "2", "0"])
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "800", "3", "0"])

    
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "200", "4", "1"])
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "400", "5", "1"])
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "800", "6", "1"])

    
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "200", "7", "2"])
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "400", "8", "2"])
    sum += subprocess.call(["./packets", "-ct", "2", "100", "32", "800", "9", "2"])
    
    sum += subprocess.call(["./packets", "-ct", "4", "100", "32", "200", "10", "0"])
    sum += subprocess.call(["./packets", "-ct", "4", "100", "32", "400", "11", "0"])
    sum += subprocess.call(["./packets", "-ct", "4", "100", "32", "800", "12", "0"])

    
    sum += subprocess.call(["./packets", "-ct", "4", "1000", "32", "200", "13", "1"])
    sum += subprocess.call(["./packets", "-ct", "4", "1000", "32", "400", "14", "1"])
    sum += subprocess.call(["./packets", "-ct", "4", "1000", "32", "800", "15", "1"])

    
    sum += subprocess.call(["./packets", "-ct", "4", "1000", "32", "200", "16", "2"])
    sum += subprocess.call(["./packets", "-ct", "4", "1000", "32", "400", "17", "2"])
    sum += subprocess.call(["./packets", "-ct", "4", "1000", "32", "800", "18", "2"])

    print("correctness test completed")
    print(f"{sum}/18 tests passed\n")


def queue_correctness_test():
    print("Running queue correctness tests")
    subprocess.call(["./test_queue", "3"])
    print("Queue correctnesss tests completed\n")

def parallel_overhead():
    print("\n\nRunning parallel overhead test")

    final_results = np.empty((3, 3), float)
    interim_results = np.empty(10, float)

    nparam = [2, 9, 14]
    wparam = [200, 400, 800]

    for i in range(0, 3):
        for j in range(0, 3):
            T = int(2**20/(nparam[i] * wparam[j]))
            for k in range(0, 10):
                subprocess.call(["./packets", "-pt", str(nparam[i]), str(T), "32", str(wparam[j]), str(k), "1"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
                with open("results.txt") as file:
                    lines = file.readlines()
                    s_time = float(lines[0])
                    p_time = float(lines[1])
                    sq_time = float(lines[2])
                interim_results[k] = sq_time/s_time
            final_results[i][j] = np.average(interim_results)

    for i in range(0, 3):
        for j in range(0,3):
            print(f"N = {nparam[i]}, W = {wparam[j]}, overhead = {final_results[i][j]}")

def dispatcher_rate():
    print("\n\nRunning dispatcher rate test")

    final_results = np.empty((6, 2), float)
    interim_results = np.empty(10, float)

    nparam = [2,3,5,9,14,28]

    for i in range(0,6):
        T = int(2**20/(nparam[i] - 1))
        for k in range(0,10):
            subprocess.call(["./packets", "-pt", str(nparam[i]), str(T), "32", "1", str(k), "1"], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
            with open("results.txt") as file:
                lines = file.readlines()
                s_time = float(lines[0])
                p_time = float(lines[1])
                sq_time = float(lines[2])
            interim_results[k] = p_time
        final_results[i][0] = T*(nparam[i] - 1)
        final_results[i][1] = np.average(interim_results)
    
    for i in range(0,6):
        print(f"N = {nparam[i]}, num threads = {final_results[i][0]}, time = {final_results[i][1]}, rate = {final_results[i][0]/final_results[i][1]}")

def speedup(type):
    print("\n\nRunning speedup test")

    final_results = np.empty((4,6), float)
    interim_results = np.empty(10, float)

    nparam = [2,3,5,9,14,28]
    wparam = [1000, 2000, 4000, 8000]

    for i in range(0, 4):
        for j in range(0, 6):
            T = 2**15
            for k in range(0, 10):
                subprocess.call(["./packets", "-pt", str(nparam[j]), str(T), "32", str(wparam[i]), str(k), str(type)], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
                with open("results.txt") as file:
                    lines = file.readlines()
                    s_time = float(lines[0])
                    p_time = float(lines[1])
                    sq_time = float(lines[2])
                interim_results[k] = s_time/p_time
            final_results[i][j] = np.average(interim_results)

    for i in range(0, 4):
        for j in range(0,6):
            print(f"W = {wparam[i]}, N = {nparam[j]}, speedup = {final_results[i][j]}")

    

def print_usage():
    print("Tester usage as follows:")
    print("python3 tester.py [options] [test_number]")
    print("options:")
    print("-ct: run the correctness test\n\t 1 will test just the queue\n\t 2 will test the algorithm")
    print("-pt: run the performance test\n\t 1 = parallel overhead \n\t 2 = dispatcher rate \n\t 3 = constant load speedup \n\t 4 = uniform load speedup \n\t 5 = exponential load speedup \n\t 6 = run all test")

def main():
    if(len(sys.argv) != 3):
        print("too few arguments")
        print_usage()
        return -1

    if(sys.argv[1] == "-ct"):
        if(sys.argv[2] == "1"):
            queue_correctness_test()
            return 1
        elif(sys.argv[2] == "2"):
            correctness_test()
            return 1
        else:
            print_usage()
            return -1
    elif(sys.argv[1] == "-pt"):
        if(sys.argv[2] == "1"):
            parallel_overhead()
            return 1
        elif(sys.argv[2] == "2"):
            dispatcher_rate()
            return 1
        elif(sys.argv[2] == "3"):
            speedup(0)
            return 1
        elif(sys.argv[2] == "4"):
            speedup(1)
            return 1
        elif(sys.argv[2] == "5"):
            speedup(2)
            return 1
        elif(sys.argv[2] == "6"):
            parallel_overhead()
            dispatcher_rate()
            speedup(0)
            speedup(1)
            speedup(2)   
        else:
            print_usage()
            return -1
    else:
        print_usage()
        return -1

main()
