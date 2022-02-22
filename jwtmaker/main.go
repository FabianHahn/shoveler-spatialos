package main

import (
  "github.com/dgrijalva/jwt-go"
  "time"
  "fmt"
  "os"
  "crypto/rsa"
  "flag"
  "io/ioutil"
)

var signKey = ``

/*
// "worker_id": Must be present. Must be a string.
// "worker_type": Must be present. Must be a string.
// "runtime_id": Must be present. Must be a string.
// "player_identity": Optional. If present, must be an object.
// "player_identity.id": Optional. If present, must be a string.
// "player_identity.provider": Optional. If present, must be a string.
// "player_identity.metadata": Optional. If present, must be a string.
*/

type TokenParams struct {
  worker_id string
  worker_type string
  runtime_id string
  pem_private_key_file string
  player_identity_id string
  player_identity_provider string
  player_identity_metadata string
}

func main() {
  var params TokenParams
  ParseParams(&params)
 
	var err error
  var private_key []byte
  private_key, err = ioutil.ReadFile(params.pem_private_key_file)
  if err != nil {
    fmt.Println(err)
    os.Exit(0)
  }

	var signedToken string
	signedToken, err = CreateToken(params, private_key)
	if err != nil {
		fmt.Println(err)
    os.Exit(0)
	} else {
		fmt.Println(signedToken)
	}
}

func ParseParams(params *TokenParams) {
  
  flag.StringVar(&params.worker_id, "worker_id", "", "The Worker ID")
  flag.StringVar(&params.worker_type, "worker_type", "", "The Worker type for the user process.")
  flag.StringVar(&params.runtime_id, "runtime_id", "",
                "The ID of the runtime the user process is connecting to.")
  flag.StringVar(&params.pem_private_key_file, "pem_private_key_file", "",
                "A .")
  flag.StringVar(&params.player_identity_id, "player_identity_id", "",
                "(optional) The ID of the player attempting the login.")
  flag.StringVar(&params.player_identity_provider, "player_identity_provider", "", 
                "(optional) The provider of the identity for player attempting the login.")
  flag.StringVar(&params.player_identity_metadata, "player_identity_metadata", "", 
                "(optional) Metadata of the identity for player attempting the login.")
   
  flag.Parse()

  // Check for mandatory fields
  if params.worker_id == "" { 
    fmt.Println("Missing mandatory parameter worker_id")
    os.Exit(1)
  }

  if params.worker_type == "" { 
    fmt.Println("Missing mandatory parameter worker_type")
    os.Exit(1)
  }

  if params.runtime_id == "" { 
    fmt.Println("Missing mandatory parameter runtime_id")
    os.Exit(1)
  }

  if params.pem_private_key_file == "" { 
    fmt.Println("Missing mandatory parameter pem_private_key_file")
    os.Exit(1)
  }
}

func CreateToken(params TokenParams, privateKeyPem []byte) (string, error) {
  tokenClaims := jwt.MapClaims{}
  tokenClaims["token_id"] = MakeTokenId()
  tokenClaims["exp"] = time.Now().Add(time.Minute * 15).Unix()
  tokenClaims["worker_id"] = params.worker_id
  tokenClaims["worker_type"] = params.worker_type
  tokenClaims["runtime_id"] = params.runtime_id

  if params.player_identity_id != "" || params.player_identity_provider != "" || params.player_identity_metadata != "" {
    player_identity := map[string]interface{}{}
    if params.player_identity_id != "" { player_identity["id"] = params.player_identity_id }
    if params.player_identity_provider != "" { player_identity["provider"] = params.player_identity_provider }
    if params.player_identity_metadata != "" { player_identity["metadata"] = params.player_identity_metadata }
    tokenClaims["player_identity"] = player_identity
  }

  var err error
  var key *rsa.PrivateKey
  key, err = jwt.ParseRSAPrivateKeyFromPEM(privateKeyPem)
  if err != nil {
	  return "", err
  }
  token := jwt.NewWithClaims(jwt.SigningMethodRS512, tokenClaims)
  signedToken, err := token.SignedString(key)
  return signedToken, err
}

func MakeTokenId() string {
  return "token_id"
}