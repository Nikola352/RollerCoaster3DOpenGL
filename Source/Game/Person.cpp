#include "../../Header/Game/Person.hpp"

Person::Person(int seatIndex)
    : seatIndex(seatIndex), hasSeatbelt(false), isSick(false)
{
}

int Person::getSeatIndex() const {
    return seatIndex;
}

bool Person::getHasSeatbelt() const {
    return hasSeatbelt;
}

bool Person::getIsSick() const {
    return isSick;
}

void Person::setHasSeatbelt(bool value) {
    hasSeatbelt = value;
}

void Person::setIsSick(bool value) {
    isSick = value;
}
