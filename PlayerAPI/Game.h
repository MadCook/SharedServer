 

class Game{
    private:
        enum State {PASSING, BIDDING, PLAYING, ROUND_OVER, GAME_OVER};
        std::vector<Card> deck;
        std::vector<Card> discardPile;
        std::vector<Player> players;
        State s;
        std::vector<Card> field;
        int turn; //index in player vector
    public:
        void initializeDeck();
        UpdateGameStateMessage(){
          for (auto&& player: players){
              player.updateField(coded message)  
          }
        }
        void deal(int numCards);
}

class Message{
    State s;
    bool turn;
    std::vector<Card> field;
}
