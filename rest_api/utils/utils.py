from datetime import datetime, timedelta
from models import TokenList, db


def cleanup_expired_tokens():
    """
    """
    expiration_limit = datetime.now() - timedelta(days=30)

    expired_tokens = TokenList.query.filter(
        TokenList.created_at < expiration_limit).all()

    if expired_tokens:
        for token in expired_tokens:
            db.session.delete(token)
        db.session.commit()
        print(f"{len(expired_tokens)} expired tokens have been deleted.")
    else:
        print("No expired tokens found.")
