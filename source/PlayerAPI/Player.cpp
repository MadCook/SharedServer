/*
* This file holds the function definitions for the Player class.
*
* Above each function is a short description of what each function does. If
* there are any questions, please do not hesitate to contact Katie Sweet or
* Ligia Frangello.
*
* -Ligia Frangello and Katie Sweet
*/

#include "Player.hpp"
#include <algorithm>
#include <iostream>

// Constructor for the Player class. Takes in the IP address of the client.
Player::Player(int id, TCPConnection::pointer connection)
  : id(id), connection(connection), 
  roundScore(0), bid(0), bags(0), tricksWon(0)
{
  std::stringstream ss;
  ss << connection->getSocket().remote_endpoint();
  ip = ss.str();
}

// Sets the name of the Player. In the future, this function will query the
// database for the name instead.
void Player::setName(std::string n)
{
  name = n;
}

// Returns the name of the player.
std::string Player::getName() const
{
  return name;
}

// Returns the id of the player.
int Player::getId()
{
  return id;
}

// Function called at the start of a round. Updates the overallScore vector with
// the previous roundScore, clears the scores and bids to get ready for the next
// round.
void Player::startNewRound()
{
  overallScore.push_back(roundScore);
  roundScore = 0;
  bid = 0;
  tricksWon = 0;
}

// Function called at the start of a new game. Updates the overallScore vector
// with the roundScore, clears the game dependent variables to get ready for a
// new game.
void Player::startNewGame()
{
  overallScore.clear();
  roundScore = 0;
  bid = 0;
  bags = 0;
  tricksWon = 0;
}


// Initializes a players hand with the 'numCards' specified.
// The deck must be passed in so that the cards can be removed from the deck
// when added to the player's hand.
void Player::initializeHand(std::vector<Card>& deck, unsigned int numCards)
{
  hand.clear();
  for (unsigned int i = 0; i < numCards; i++)
  {
    hand.push_back(deck.back());
    deck.pop_back();
  }
  std::sort(hand.begin(), hand.end());
}

// Inserts card into the hand in order.
void Player::insertCardToHand(const Card& c)
{
  hand.push_back(c);
  std::sort(hand.begin(), hand.end());
}

// Attempts to remove card from hand. If card is in hand, it will be removed
// and the function will return true. If the card is not in the hand, it will
// return false and no cards will be removed.
bool Player::removeCardFromHand(const Card& c)
{
  for (auto i = hand.begin(); i < hand.end(); i++)
  {
    if (c == *i)
    {
      hand.erase(i);
      return true;
    }
  }
  return false;
}

// Returns a vector of Cards that matches the Player's hand.
std::vector<Card> Player::getHand() const
{
  return hand;
}

// Returns a players score the current round.
int Player::getRoundScore() const
{
  return roundScore;
}

// Allows for a players round score to be sent. Anticipated to be unnecessary
// since the round score will be set to 0 automatically when endTheRound()
// is called, and the incrementRoundScore function below will allow for an
// increase in the round score after a particular trick finishes.
void Player::setRoundScore(int i)
{
  roundScore = i;
}

// Allows for a players round score to be increased by the integer specified.
// For use in games like Hearts where a player's round score may be updated
// multiple times in one round (i.e. after a trick is assigned).
void Player::incrementRoundScore(int i)
{
  roundScore += i;
}

// Returns a vector of scores for each player. Each entry in the vector
// corresponds to the player's score in a different round within the game.
std::vector<int> Player::getOverallScores() const
{
  return overallScore;
}

// Returns the sum of the player's round scores (i.e. their total score)
int Player::getTotalScore() const
{
  int totalScore = 0;
  for (auto&& score : overallScore)
  {
    totalScore += score;
  }
  totalScore += roundScore;
  return totalScore;
}

// Gets the value that a player bid (Spades).
int Player::getBid() const
{
  return bid;
}

void Player::setBid(int b)
{
  bid = b;
}

// Gets the number of bags a player had (Spades).
int Player::getBags() const
{
  return bags;
}

// Sets the number of bags a player had (Spades).
void Player::setBags(int b)
{
  bags = b;
}

// Gets the number of tricks a player has won in a round (Spades).
int Player::getTricksWon() const
{
  return tricksWon;
}

// Sets the number of tricks a player has won in a round (Spades).
void Player::setTricksWon(int t)
{
  tricksWon = t;
}

// Increments the number of tricks a player has won in a round (Spades).
// Allows for the number of tricks won to be updated after every trick.
void Player::incrementTricksWon()
{
  tricksWon++;
}

void Player::setValidateSuit(std::function<void(Suit)> func)
{
  validateSuit = func;
}

void Player::setValidateMove(std::function<void(Card)> func)
{
  validateMove = func;
}

void Player::setValidateBid(std::function<void(int)> func)
{
  validateBid = func;
}

bool operator==(const Player& p1, const Player& p2)
{
  if (p1.hand.size() != p2.hand.size()) return false;
  for (auto i = 0u; i < p1.hand.size(); i++)
  {
    if (!(p1.hand[i] == p2.hand[i])) return false;
  }
  return (p1.id == p2.id && p1.ip.compare(p2.ip) == 0 &&
          p1.name.compare(p2.name) == 0);
}

std::ostream& operator<<(std::ostream& out, const Player& p)
{
  out << p.id << ", " << p.ip << ", " << p.name;
  return out;
}

void Player::requestMove()
{
  connection->write("Give Move");
  connection->aSyncRead(boost::bind(&Player::receivedMove, this, _1));
}

void Player::requestBid()
{
  connection->write("Give Bid");
  connection->aSyncRead(boost::bind(&Player::receivedBid, this, _1));
}

void Player::requestSuit()
{
  connection->write("Give Suit");
  connection->aSyncRead(boost::bind(&Player::receivedSuit, this, _1));
}

void Player::updateGameStatus()
{
  connection->write("Status Update");
  connection->write(""/*List of cards and players*/);
}

void Player::readMessage()
{
  connection->aSyncRead(boost::bind(&Player::recivedMessage,this,_1));
}

// hellper to decode the suit from a char.
Suit decodeSuit(char s) {
  Suit suit;
  if (s == 'H') {
    suit = HEARTS;
  }
  else if (s == 'C') {
    suit = CLUBS;
  }
  else if (s == 'D') {
    suit = DIAMONDS;
  }
  else if (s == 'S') {
    suit = SPADES;
  }
  else {
    suit = UNDEFINED;
  }
  return suit;
}

void Player::receivedMove(std::string msg)
{
  // we assume the message format is <char suit>{<char><optional char>number} ex H2 = 2 of hearts.
  Suit suitCode = decodeSuit(msg.at(0));
  Value cardNumber = Value(std::stoi(msg.substr(1)));

  Card c = Card(suitCode, cardNumber);
  validateMove(c);
}

void Player::receivedBid(std::string msg)
{
  // We assume that no other information other than the bid as parsable number from a 
  // string is given.
  int bid = std::stoi(msg);
    
  validateBid(bid);
}

void Player::receivedSuit(std::string msg)
{
  // so we assume the first char of the recieved message will be the suit, either
  // H, S, C, D, all other input will be interpreted as undefined suit.
  Suit s = decodeSuit(msg[0]);
  
  validateSuit(s);
}

void Player::recivedMessage(std::string msg)
{
  std::cout << "Message from : " << *this << std::endl;
  std::cout << msg << std::endl;
  readMessage();
}

