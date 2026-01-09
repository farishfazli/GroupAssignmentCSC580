#include <mpi.h>
#include <stdio.h>

#define STUDENTS 10
#define PROGRAMS 5

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, world;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world);

    // 10x5 dataset: rows=students, cols=programs
    // Cols: 0=CS, 1=IT, 2=SE, 3=DS, 4=Cyber
    int examResults[STUDENTS][PROGRAMS] = {
        {78, 82, 80, 85, 83},  // Student 1
        {65, 70, 68, 72, 71},  // Student 2
        {92, 90, 91, 94, 93},  // Student 3
        {67, 69, 70, 71, 68},  // Student 4
        {74, 76, 75, 78, 77},  // Student 5
        {88, 86, 87, 90, 89},  // Student 6
        {90, 91, 92, 93, 94},  // Student 7
        {69, 72, 71, 73, 70},  // Student 8
        {81, 83, 82, 85, 84},  // Student 9
        {95, 93, 94, 96, 97}   // Student 10
    };

    const char *programNames[PROGRAMS] = {
        "Computer Science", "Information Tech", "Software Eng", "Data Science", "Cyber Security"
    };

    // Divide rows among processes (block distribution)
    int base = STUDENTS / world;
    int rem  = STUDENTS % world;

    int start = rank * base + (rank < rem ? rank : rem);
    int count = base + (rank < rem ? 1 : 0);
    int end   = start + count; // exclusive

    // Local best student: store as (value=sum, index=studentIndex)
    // We'll use MPI_MAXLOC on MPI_2INT where:
    //   struct-like: {value, index}
    int localBest[2] = { -1, -1 };

    // Local program sums
    int localProgSum[PROGRAMS] = {0};

    // Compute local results
    for (int i = start; i < end; i++) {
        int sum = 0;
        for (int j = 0; j < PROGRAMS; j++) {
            sum += examResults[i][j];
            localProgSum[j] += examResults[i][j];
        }

        if (sum > localBest[0]) {
            localBest[0] = sum;
            localBest[1] = i; // store 0-based student index
        }
    }

    // Reduce to global best student (MAXLOC)
    int globalBest[2] = { -1, -1 };
    MPI_Reduce(localBest, globalBest, 1, MPI_2INT, MPI_MAXLOC, 0, MPI_COMM_WORLD);

    // Reduce program sums (SUM)
    int globalProgSum[PROGRAMS] = {0};
    MPI_Reduce(localProgSum, globalProgSum, PROGRAMS, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int bestStudentIndex = globalBest[1]; // 0-based
        int bestStudentTotal = globalBest[0];

        // Find best program by highest average
        int bestProgIndex = 0;
        double bestProgAvg = (double)globalProgSum[0] / STUDENTS;

        for (int j = 1; j < PROGRAMS; j++) {
            double avg = (double)globalProgSum[j] / STUDENTS;
            if (avg > bestProgAvg) {
                bestProgAvg = avg;
                bestProgIndex = j;
            }
        }

        printf("=== MPI RESULTS (Student Examination Result Analysis) ===\n\n");

        printf("Output 1: Best Student\n");
        printf("Best Student = Student %d\n", bestStudentIndex + 1);
        printf("Total Marks  = %d\n\n", bestStudentTotal);

        printf("Output 2: Best Program\n");
        printf("Best Program = %s\n", programNames[bestProgIndex]);
        printf("Average Mark = %.2f\n\n", bestProgAvg);

        printf("Program Averages:\n");
        for (int j = 0; j < PROGRAMS; j++) {
            printf("- %s: %.2f\n", programNames[j], (double)globalProgSum[j] / STUDENTS);
        }
    }

    MPI_Finalize();
    return 0;
}