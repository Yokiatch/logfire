from fastapi import FastAPI
from python.api.routes.query import router as query_router

app = FastAPI(
    title="logfire",
    description="High-performance log analytics engine — C++ core, Python API",
    version="0.1.0",
)

app.include_router(query_router, prefix="/query", tags=["query"])

@app.get("/health")
def health():
    return {"status": "ok"}