#include <iostream>
#include <thread>
#include <vector>

#include <stdio.h>
#include <Windows.h>


using namespace std;

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr; // dynamically allocating space for playerfield

// command line als screen buffer gebruiken

int nScreenWidth = 80;
int nScreenHeight = 30;


int rotate(int px, int py, int r)
{
    switch (r % 4)
    {
    case 0: return py * 4 + px; // 0 degrees
    case 1: return 12 + py - (px * 4); // 90
    case 2: return 15 - (py * 4) - px; // 180
    case 3: return 3 - py + (px * 4); // 270
    }
    return 0;
}


bool doesPieceFit(int nTetramino, int rotation, int nPosX, int nPosY) // posx and posy is linksboven stuk van de tetramino
{
    for(int px = 0; px <4; px++)
        for (int py = 0; py < 4; py++) 
        {
            // welke piece is t
            int pi = rotate(px, py, rotation);

            // waar in eht field is het
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            // checken of we in bounds zijn
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) 
            {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) 
                {
                    if (tetromino[nTetramino][pi] != L'.' && pField[fi] != 0)
                    {
                        return false; // fail als t 1 keer al niet werkt
                    }
                }
            }
        }
    return true;
}


int main()
{
    // generate assets
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"....");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XXX");
    tetromino[3].append(L"..X.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"..X.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"..XX");
    tetromino[4].append(L"....");

    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"....");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");

    pField = new unsigned char[nFieldWidth * nFieldHeight]; // playerfield gennen
    for (int x = 0; x < nFieldWidth; x++)
    {
        for (int y = 0; y < nFieldHeight; y++)
        {
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
        }
    }

    wchar_t* screen = new wchar_t[nScreenHeight * nScreenWidth]; // nScreenWidth * nScreenHeight
    for (int i = 0; i < 25000; i++) screen[i] = L' '; // wat de kkr 
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole); // nu maken we een screen buffer die niet naar cout luisterd
    DWORD dwBytesWritten = 0;

    // gamestate
    bool bGameOver = false;

    // dit spawned een blokje middenboven
    int nCurrentPiece = 0;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    int nSpeed = 20; // pretty much de difficulty setting
    int nSpeedCounter = 0; 
    bool bForceDown = false;
    int pieceCount = 0;
    int score = 0;

    vector<int> vLines;

    // keys pakken
    bool bKey[4]; // we hebben maar 4 keys
    bool bRotateHold = false;

    while (!bGameOver)
    {
        // game timing ===============
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed); // als de counter gelijk is aan de speed moet t blokje omlaag

        // input =====================
        for (int i = 0; i < 4; i++)                                //hex for r l d en z
            bKey[i] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[i]))) != 0;

        // game logic ================
        nCurrentX += (bKey[0] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0; // if is hier weggewerkt
        nCurrentX -= (bKey[1] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[2] && doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))? 1: 0;// plus 1 linksboven is 0,0

        if (bKey[3]) {
            nCurrentRotation += (!bRotateHold && doesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY));
            bRotateHold = true;
        }
        else  
            bRotateHold = false; 

        if (bForceDown) 
        {
            if(doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) // y+ 1 want computers zijn gay
                nCurrentY++;
            else
            {
                // lock currentpiece in field
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                pieceCount++;
                if (pieceCount % 10 == 0)
                    if (nSpeed >= 10) nSpeed--;

                // do we have a horizontal line
                for(int py = 0; py < 4; py++)
                    if (nCurrentY + py < nFieldHeight - 1) 
                    {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                        if (bLine) 
                        {
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8; // 8 is de = sign

                            vLines.push_back(nCurrentY + py);
                        }
                    }
                score += 25;
                if (!vLines.empty()) score += (1 << vLines.size()) * 100; // if tetris dan 1600

                // get next piece
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;
                //if piece does not fit game over
                bGameOver = !doesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
            nSpeedCounter = 0;
        }

        // render output =============

        //draw field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++)
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

        // draw first piece
        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++)
                if (tetromino[nCurrentPiece][rotate(px, py, nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65; // 65 voor ascii

        //draw score buffer moet 9 zijn want de score string is static
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"score: %d", score);// buffer buffercount de variable en de formatting

        if(!vLines.empty())
        {
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (auto &v : vLines)
                for (int px = 1; px < nFieldWidth - 1; px++) 
                {
                    for (int py = v; py > 0; py--)
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0;
                }
            vLines.clear();
        }

        // display frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }
    CloseHandle(hConsole);
    cout << "game over skill issue Score: " << score << endl;
    system("pause");
    return 0;
}