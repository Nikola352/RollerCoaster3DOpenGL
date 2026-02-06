#pragma once

class Person {
private:
    int seatIndex;
    bool hasSeatbelt;
    bool isSick;

public:
    Person(int seatIndex);

    int getSeatIndex() const;
    bool getHasSeatbelt() const;
    bool getIsSick() const;

    void setHasSeatbelt(bool value);
    void setIsSick(bool value);
};
