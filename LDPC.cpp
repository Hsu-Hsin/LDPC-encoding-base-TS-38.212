#include <iostream>
#include <math.h>

/*array form of the polynomial to calculate CRC parity*/
const bool gCRC24A[25] = {1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,
                          1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1};
const bool gCRC24B[25] = {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1};

/*length of CRC polynomial*/
const int LENGTH_POLYNOMIAL = 25;
const int LENGTH_CRC = 24;

const int Kcb = 8448;

void CRC(bool input_arr[], int length_input_arr, const bool polynomial_arr[],
         int length_crc, bool output_arr[]) {
    bool temp_arr[length_input_arr + length_crc] = {0};
    for (int i = 0; i < length_input_arr; i++) {
        temp_arr[i] = input_arr[i];
    }
    for (int i = length_input_arr; i < length_input_arr + length_crc; i++) {
        temp_arr[i] = 0;
    }

    int mark = 0;
    while (mark != length_input_arr) {
        for (int i = 0; i < length_crc + 1; i++) {
            temp_arr[i + mark] = temp_arr[i + mark] ^ polynomial_arr[i];
        }
        int i = mark;
        while (temp_arr[i] == 0 && mark < length_input_arr) {
            mark++;
            i++;
        }
    }

    for (int i = 0; i < length_crc; i++) {
        output_arr[i] = temp_arr[i + length_input_arr];
    }
}


void CodeBlcokSegmentation(bool input_arr[], int C, int K_apostrophe, int L,
                           int K, short output_arr[][Kcb]) {
    int s = 0;
    bool crc_arr[K_apostrophe - L] = {0};
    bool pr_arr[L] = {0};
    //std::cout << "CodeBlcokSegmentation" << std::endl;
    for (int r = 0; r < C; r++) {
        //std::cout << "CodeBlcokSegmentation: " << r << std::endl;
        int i = 0;
        for (int k = 0; k < K_apostrophe - L - 1; k++) {
            output_arr[r][k] = input_arr[s];
            crc_arr[i] = input_arr[s];
            s++;
            i++;
        }
        if (C > 1) {
            /*
             * calculate CRC parity bit for every code block
             */
            CRC(crc_arr, K_apostrophe - L, gCRC24B, LENGTH_CRC, pr_arr);
            for (int k = K_apostrophe - L; k < K_apostrophe; k++)
                // crk[r][k] = p[r][k + L - K1];
                output_arr[r][k] = pr_arr[k + L - K_apostrophe];
        }

        for (int k = K_apostrophe; k < K; k++) {
            output_arr[r][k] = -1;  // how to express a NaN number
            //std::cout << output_arr[r][k];
        }
        //std::cout << std:: endl;
    }
}

int main() {
    /*
     * FUNCTION: input parameters
     * LENGTH_BIT_STREAM = A = 1179864
     * BASEGRAPH = 1 when base graph 1 or 2 when base graph 2
     */
    const int LENGTH_BIT_STREAM = 1179864;
    const int BASEGRAPH = 1;
    const int B = LENGTH_BIT_STREAM + LENGTH_CRC;
    freopen("./codeblocksegmentation.txt", "w", stdout);
    /*
     * FUNCTION: random bit stream generation
     * INPUT: length of random bit stream to be generated
     * OUTPUT: bool array random_bit_stream[] of random bit stream
     *         a0, a1, a2, ... , a(A-1)
     */
    bool random_bit_stream[LENGTH_BIT_STREAM] = {0};
    srand(time(nullptr));
    for (int i = 0; i < LENGTH_BIT_STREAM; i++) {
        random_bit_stream[i] = rand() % 2;
    }

    /*
     * FUNCTION: CRC calculation
     * INPUT: bool array of random bit stream,
     *        length of CRC polynomial, LENGTH_POLYNOMIAL, which is L
     * OUTPUT: bool array CRC_arr[] that has been added the gCRC24A parity
     *         b0, b1, b2, ... ,b(L-1)
     */
    bool CRC_arr[LENGTH_CRC] = {0};
    CRC(random_bit_stream, LENGTH_BIT_STREAM, gCRC24A, LENGTH_CRC, CRC_arr);

    /*
     * Because the first length_input_arr bit is set to zero when XOR
     * We need output
     * a0, a1, a2, ... ,a(A-1), b0, b1, b2, ... ,b(L-1)
     * b0, b1, b2, ... ,b(A-1), b(A), b(A+1), ... ,b(B-1). where B = A + L
     */
    bool CRC_output_arr[LENGTH_BIT_STREAM + LENGTH_CRC] = {0};
    for (int i = 0; i < LENGTH_BIT_STREAM; i++) {
        CRC_output_arr[i] = random_bit_stream[i];
    }
    for (int i = LENGTH_BIT_STREAM; i < LENGTH_BIT_STREAM + LENGTH_CRC; i++) {
        CRC_output_arr[i] = CRC_arr[i - LENGTH_BIT_STREAM];
    }

    std::cout << "bit stream: " << std::endl;
    for(int i = 0; i < LENGTH_BIT_STREAM; i++)
    {
        std::cout << CRC_output_arr[i] ;
    }
    std::cout << std::endl;
    std::cout << "CRC: " << std::endl;
    for(int i = LENGTH_BIT_STREAM; i < LENGTH_BIT_STREAM + LENGTH_CRC; i++)
    {
        std::cout << CRC_output_arr[i] ;
    }
    std::cout << std::endl;
    /*
     * FUNCTION: code block segmentation and code block CRC attachment
     * INPUT: bool array that has been added the gCRC24A parity
     * OUTPUT: bool array code_block_CRC_arr[][] that has been segmented and CRC
     * attached
     */

    /*
     * get Kcb, Kb and L according base graph
     * calculate: code block number C
     *            B', K', Zc and K
     * named Kcb, Kb, L_temp, CodeBlockNum, B_temp, K_temp, Zc and BitNum_EveryCodeBlock
     */
    std::cout << "B = " << B << std::endl;
    int Kb = 0;
    int CodeBlockNum = 0;
    int B_apostrophe = 0;
    //int K_temp = 8392;
    int BitNum_EveryCodeBlock = 0;

    /*
     * calculate Kcb
    if (BASEGRAPH == 1)
        Kcb = 8448;
    if (BASEGRAPH == 2)
        Kcb = 3840;*/
    std::cout << "Kcb = " << Kcb << std::endl;
    /*calculate code block number C and L*/
    int L_temp = 0;
    if (LENGTH_BIT_STREAM + LENGTH_CRC <= Kcb){
        L_temp = 0;
        CodeBlockNum = 1;
        B_apostrophe = LENGTH_BIT_STREAM + L_temp;
    } else {
        L_temp = 24;
        CodeBlockNum = ceil((LENGTH_BIT_STREAM + LENGTH_CRC) / (float)(Kcb - L_temp));
        B_apostrophe = LENGTH_BIT_STREAM + LENGTH_CRC + CodeBlockNum * L_temp;
    }
    std::cout << "code block number = " << CodeBlockNum << std::endl;
    std::cout << "L = " << L_temp << std::endl;
    std::cout << "B' = " << B_apostrophe << std::endl;
    /*calculate the number of bits K in each code block*/
    float K_apostrophe = B_apostrophe / (float)CodeBlockNum;
    std::cout << "K' = " << K_apostrophe << std::endl;
    if (BASEGRAPH == 1){
        Kb = 22;
    }
    if (BASEGRAPH == 2){
        if (B > 640)
            Kb = 10;
        else if (B > 560)
            Kb = 9;
        else if (B > 192)
            Kb = 8;
        else
            Kb = 6;
    }
    std::cout << "Kb = " << Kb << std::endl;
    int Zc_arr[8][8] = {
            {2, 4, 8, 16, 32, 64, 128, 256},
            {3, 6, 12, 24, 48, 96, 192, 384},
            {5, 10, 20, 40, 80, 160, 320},
            {7, 14, 28, 56, 112, 224},
            {9, 18, 36, 72, 144, 288},
            {11, 22, 44, 88, 176, 352},
            {13, 26, 52, 104, 208},
            {15, 30, 60, 120, 240}
    };
    int Zc_val = 1000;
    float temp = K_apostrophe / (float)Kb;
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            if (Zc_arr[i][j] >= temp && Zc_arr[i][j] < Zc_val)
                Zc_val = Zc_arr[i][j];
        }
    }
    std::cout << "Zc = " << Zc_val << std::endl;
    if (BASEGRAPH == 1)
        BitNum_EveryCodeBlock = 22 * Zc_val;
    if (BASEGRAPH == 2)
        BitNum_EveryCodeBlock = 10 * Zc_val;
    std::cout << "K = " << BitNum_EveryCodeBlock << std::endl;
    //int Kcb = 8448;
    short CodeBlock[CodeBlockNum][Kcb] = {0};

    CodeBlcokSegmentation(CRC_output_arr, CodeBlockNum, (int)K_apostrophe, LENGTH_CRC,
                          BitNum_EveryCodeBlock, CodeBlock);

    std:: cout << "final output: " << std::endl;
    for (int i = 0; i < CodeBlockNum; i++){
        for (int j = 0; j < BitNum_EveryCodeBlock; j++){
            std::cout << CodeBlock[i][j];
            if ((j + 1) % ((int)K_apostrophe - LENGTH_CRC) == 0 || (j + 1) % ((int)K_apostrophe) == 0)
                std::cout << "   ###   ";
        }
        std::cout << std::endl;
    }

    freopen("./OUTPUT.txt", "w", stdout);
    for (int i = 0; i < CodeBlockNum; i++){
        for (int j = 0; j < BitNum_EveryCodeBlock; j++){
            std::cout << CodeBlock[i][j];
        }
        std::cout << std::endl;
    }

    return 0;
}
