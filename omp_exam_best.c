#include <omp.h>
#include <stdio.h>

#define STUDENTS 10
#define PROGRAMS 5

int main() {
    int examResults[STUDENTS][PROGRAMS] = {
        {78, 82, 80, 85, 83},
        {65, 70, 68, 72, 71},
        {92, 90, 91, 94, 93},
        {67, 69, 70, 71, 68},
        {74, 76, 75, 78, 77},
        {88, 86, 87, 90, 89},
        {90, 91, 92, 93, 94},
        {69, 72, 71, 73, 70},
        {81, 83, 82, 85, 84},
        {95, 93, 94, 96, 97}
    };

    const char *programNames[PROGRAMS] = {
        "Computer Science", "Information Tech", "Software Eng", "Data Science", "Cyber Security"
    };

    int globalBestStudentIndex = -1;
    int globalBestStudentTotal = -1;

    int globalProgSum[PROGRAMS] = {0};

    // Choose threads (you can change this)
    omp_set_num_threads(4);

    printf("=== OpenMP RESULTS (Student Examination Result Analysis) ===\n");
    printf("Threads used: %d\n\n", omp_get_max_threads());

    #pragma omp parallel
    {
        int localBestIndex = -1;
        int localBestTotal = -1;

        int localProgSum[PROGRAMS] = {0};

        #pragma omp for
        for (int i = 0; i < STUDENTS; i++) {
            int sum = 0;
            for (int j = 0; j < PROGRAMS; j++) {
                sum += examResults[i][j];
                localProgSum[j] += examResults[i][j];
            }

            if (sum > localBestTotal) {
                localBestTotal = sum;
                localBestIndex = i;
            }
        }

        // Merge thread results safely
        #pragma omp critical
        {
            // best student
            if (localBestTotal > globalBestStudentTotal) {
                globalBestStudentTotal = localBestTotal;
                globalBestStudentIndex = localBestIndex;
            }

            // program sums
            for (int j = 0; j < PROGRAMS; j++) {
                globalProgSum[j] += localProgSum[j];
            }
        }
    }

    // Find best program by average
    int bestProgIndex = 0;
    double bestProgAvg = (double)globalProgSum[0] / STUDENTS;

    for (int j = 1; j < PROGRAMS; j++) {
        double avg = (double)globalProgSum[j] / STUDENTS;
        if (avg > bestProgAvg) {
            bestProgAvg = avg;
            bestProgIndex = j;
        }
    }

    printf("Output 1: Best Student\n");
    printf("Best Student = Student %d\n", globalBestStudentIndex + 1);
    printf("Total Marks  = %d\n\n", globalBestStudentTotal);

    printf("Output 2: Best Program\n");
    printf("Best Program = %s\n", programNames[bestProgIndex]);
    printf("Average Mark = %.2f\n\n", bestProgAvg);

    printf("Program Averages:\n");
    for (int j = 0; j < PROGRAMS; j++) {
        printf("- %s: %.2f\n", programNames[j], (double)globalProgSum[j] / STUDENTS);
    }

    return 0;
}
